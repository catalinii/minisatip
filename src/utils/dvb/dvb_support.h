#include <stdio.h>
#include <stdint.h>

/*
 * DVB String conversion according to EN 300 468, Annex A
 * Not all character sets are supported, but it should cover most of them
 */
int dvb_get_string(char *dst, size_t dstlen, const uint8_t *src, size_t srclen);
