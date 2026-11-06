#include <stdint.h>
#include <stdio.h>

/*
 * DVB String conversion according to EN 300 468, Annex A
 * Not all character sets are supported, but it should cover most of them
 */
int dvb_get_string(char *dst, size_t dstlen, const uint8_t *src, size_t srclen);

/* Charset override for dvb_get_string (see --sdt-charset): raw Latin-1
 * without a prefix, which ISO-6937 mangles (issue #1477). */
#define DVB_CHARSET_AUTO 0
#define DVB_CHARSET_LATIN1 1
void dvb_set_charset_override(int charset);
