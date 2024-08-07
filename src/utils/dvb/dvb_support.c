/*
 *  The functions in this file are copied from the tvheadend project 
 * (https://github.com/tvheadend/tvheadend) and the copyright notice below is 
 *  taken from there:
 * 
 *  TV Input - DVB - Support/Conversion functions
 *  Copyright (C) 2007 Andreas Öman
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dvb_support.h"
#include "dvb_charset_tables.h"

#include <errno.h>
#include <sys/types.h>

static int convert_iso_8859[16] = {
  -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, -1, 11, 12, 13
};

#define convert_utf8   14
#define convert_iso6937 15
#define convert_ucs2 16

#define conv_lower(dst, dstlen, c) \
  if (c >= 0x20 || c == '\n') { *(dst++) = c; (*dstlen)--; }

static inline int encode_utf8(unsigned int c, char *outb, int outleft)
{
  if (c <= 0x7F && outleft >= 1) {
    if (c >= 0x20 || c == '\n') {
      *outb = c;
      return 1;
    }
    return 0;
  } else if (c <= 0x7FF && outleft >=2) {
    *outb++ = ((c >>  6) & 0x1F) | 0xC0;
    *outb++ = ( c        & 0x3F) | 0x80;
    return 2;
  } else if (c <= 0xFFFF && outleft >= 3) {
    *outb++ = ((c >> 12) & 0x0F) | 0xE0;
    *outb++ = ((c >>  6) & 0x3F) | 0x80;
    *outb++ = ( c        & 0x3F) | 0x80;
    return 3;
  } else if (c <= 0x10FFFF && outleft >= 4) {
    *outb++ = ((c >> 18) & 0x07) | 0xF0;
    *outb++ = ((c >> 12) & 0x3F) | 0x80;
    *outb++ = ((c >>  6) & 0x3F) | 0x80;
    *outb++ = ( c        & 0x3F) | 0x80;
    return 4;
  } else {
    return -1;
  }
}

static inline size_t conv_utf8(const uint8_t *src, size_t srclen,
                               char *dst, size_t *dstlen)
{
  while (srclen>0 && (*dstlen)>0) {
    uint_fast8_t c = *src;
    if (c <= 0x7f) {
      conv_lower(dst, dstlen, c);
    } else {
      *(dst++) = c;
      (*dstlen)--;
    }
    srclen--;
    src++;
  }
  if (srclen>0) {
    errno = E2BIG;
    return -1;
  }
  return 0;
}

static inline size_t conv_6937(const uint8_t *src, size_t srclen,
                              char *dst, size_t *dstlen)
{
  while (srclen>0 && (*dstlen)>0) {
    uint_fast8_t c = *src;
    if (c <= 0x7f) {
      conv_lower(dst, dstlen, c);
    } else if (c <= 0x9f) {
      // codes 0x80 - 0x9f (control codes) are ignored except CR/LF
      if (c == 0x8a) {
        *dst = '\n';
        (*dstlen)--;
        dst++;
      }
    } else {
      uint16_t uc;
      if (c >= 0xc0 && c <= 0xcf) {
        // map two-byte sequence, skipping illegal combinations.
        if (srclen<2) {
          errno = EINVAL;
          return -1;
        }
        srclen--;
        src++;
        uint8_t c2 = *src;
        if (c2 == 0x20) {
          uc = iso6937_lone_accents[c-0xc0];
        } else if (c2 >= 0x41 && c2 <= 0x5a) {
          uc = iso6937_multi_byte[c-0xc0][c2-0x41];
        } else if (c2 >= 0x61 && c2 <= 0x7a) {
          uc = iso6937_multi_byte[c-0xc0][c2-0x61+26];
        } else {
          uc = 0;
        }
      } else {
        // map according to single character table, skipping
        // unmapped chars (value 0 in the table)
        uc = iso6937_single_byte[c-0xa0];
      }
      if (uc != 0) {
        int len = encode_utf8(uc, dst, *dstlen);
        if (len == -1) {
          errno = E2BIG;
          return -1;
        } else {
          (*dstlen) -= len;
          dst += len;
        }
      }
    }
    srclen--;
    src++;
  }
  if (srclen>0) {
    errno = E2BIG;
    return -1;
  }
  return 0;
}

static inline size_t conv_UCS2(const uint8_t *src, size_t srclen,char *dst, size_t *dstlen)
{
  while (srclen>0 && (*dstlen)>0){
    uint16_t uc = *src<<8|*(src+1);
    if (uc >= 0xe080 && uc <= 0xe09f) {
      // codes 0xe080 - 0xe09f (control codes) are ignored except CR/LF
      if (uc == 0xe08a) {
        *dst = '\n';
        (*dstlen)--;
        dst++;
      }
    } else {
      int len = encode_utf8(uc, dst, *dstlen);
      if (len == -1) {
        errno = E2BIG;
        return -1;
      } else {
        (*dstlen) -= len;
        dst += len;
      }
    }
    srclen-=2;
    src+=2;
  }
  if (srclen>0) {
    errno = E2BIG;
    return -1;
  }
  return 0;
}

static inline size_t conv_8859(int conv,
                              const uint8_t *src, size_t srclen,
                              char *dst, size_t *dstlen)
{
  uint16_t *table = conv_8859_table[conv];

  while (srclen>0 && (*dstlen)>0) {
    uint_fast8_t c = *src;
    if (c <= 0x7f) {
      conv_lower(dst, dstlen, c);
    } else if (c <= 0x9f) {
      // codes 0x80 - 0x9f (control codes) are ignored except CR/LF
      if (c == 0x8a) {
        *dst = '\n';
        (*dstlen)--;
        dst++;
      }
    } else {
      // map according to character table, skipping
      // unmapped chars (value 0 in the table)
      uint_fast16_t uc = table[c-0xa0];
      if (uc != 0) {
        int len = encode_utf8(uc, dst, *dstlen);
        if (len == -1) {
          errno = E2BIG;
          return -1;
        } else {
          (*dstlen) -= len;
          dst += len;
        }
      }
    }
    srclen--;
    src++;
  }
  if (srclen>0) {
    errno = E2BIG;
    return -1;
  }
  return 0;
}

static inline size_t dvb_convert(int conv,
                          const uint8_t *src, size_t srclen,
                          char *dst, size_t *dstlen)
{
  switch (conv) {
    case convert_utf8: return conv_utf8(src, srclen, dst, dstlen);
    case convert_iso6937: return conv_6937(src, srclen, dst, dstlen);
    case convert_ucs2:return conv_UCS2(src,srclen,dst,dstlen);
    default: return conv_8859(conv, src, srclen, dst, dstlen);
  }
}

int
dvb_get_string
  (char *dst, size_t dstlen, const uint8_t *src, size_t srclen)
{
  int ic = -1;
  size_t len, outlen;

  if(srclen < 1) {
    *dst = 0;
    return 0;
  }

  // automatic charset detection
  switch(src[0]) {
  case 0:
    *dst = 0; // empty string (confirmed!)
    return 0;

  case 0x01 ... 0x0b: /* ISO 8859-X */
    ic = convert_iso_8859[src[0] + 4];
    src++; srclen--;
    break;

  case 0x0c ... 0x0f: /* reserved for the future use */
    src++; srclen--;
    break;

  case 0x10: /* ISO 8859 - Table A.4 */
    if(srclen < 3 || src[1] != 0 || src[2] == 0 || src[2] > 0x0f)
      return -1;

    ic = convert_iso_8859[src[2]];
    src+=3; srclen-=3;
    break;
    
  case 0x11: /* ISO 10646 */
    ic = convert_ucs2;
    src++; srclen--;
    break;

  case 0x12: /* KSX1001-2004 - Korean Character Set - NYI! */
    src++; srclen--;
    break;

  case 0x13: /* NYI */
    src++; srclen--;
    break;

  case 0x14: /* Big5 subset of ISO 10646 */
    ic = convert_ucs2;
    src++; srclen--;
    break;

  case 0x15: /* UTF-8 */
    ic = convert_utf8;
    src++; srclen--;
    break;

  case 0x16 ... 0x1e: /* reserved for the future use */
    src++; srclen--;
    break;

  case 0x1f: /* Described by encoding_type_id, TS 101 162 */
    return -1; /* NYI */

  default:
    ic = convert_iso6937;
    break;
  }

  if(srclen < 1) {
    *dst = 0;
    return 0;
  }

  if(ic == -1)
    return -1;

  outlen = dstlen - 1;

  if (dvb_convert(ic, src, srclen, dst, &outlen) == -1)
    return -1;

  len = dstlen - outlen - 1;
  dst[len] = 0;
  return 0;
}
