/*
 * DVB charset conversion tests: the --sdt-charset latin1 override must
 * decode raw Latin-1 SDT names (issue #1477, NOS Portugal), while the
 * auto mode keeps the DVB prefix detection unchanged.
 */
#include "opts.h"
#include "utils/dvb/dvb_support.h"
#include "utils/testing.h"
#include <stdio.h>
#include <string.h>

int test_charset_latin1_override() {
    char out[64];
    // "RTP Noticias" with raw Latin-1 i (0xED), no DVB prefix
    const uint8_t latin1[] = "RTP Not\xED"
                             "cias";
    dvb_set_charset_override(DVB_CHARSET_LATIN1);
    ASSERT(dvb_get_string(out, sizeof(out), latin1, sizeof(latin1) - 1) == 0,
           "latin1 name should decode");
    ASSERT(strcmp(out, "RTP Not\xC3\xAD"
                       "cias") == 0,
           "latin1 0xED should become U+00ED");
    // "Localvisao" with raw Latin-1 a-tilde (0xE3)
    const uint8_t local[] = "Localvis\xE3"
                            "o";
    ASSERT(dvb_get_string(out, sizeof(out), local, sizeof(local) - 1) == 0,
           "latin1 name should decode");
    ASSERT(strcmp(out, "Localvis\xC3\xA3"
                       "o") == 0,
           "latin1 0xE3 should become U+00E3");
    dvb_set_charset_override(DVB_CHARSET_AUTO);
    return 0;
}

int test_charset_auto_keeps_prefix_detection() {
    char out[64];
    dvb_set_charset_override(DVB_CHARSET_AUTO);
    // explicit UTF-8 still wins in auto mode
    const uint8_t utf8[] = "\x15Not\xC3\xAD"
                           "cias";
    ASSERT(dvb_get_string(out, sizeof(out), utf8, sizeof(utf8) - 1) == 0,
           "utf8 name should decode");
    ASSERT(strcmp(out, "Not\xC3\xAD"
                       "cias") == 0,
           "utf8 prefix should pass through");
    // explicit Latin-1 via the Table A.4 escape (0x10 0x00 0x01)
    const uint8_t esc[] = "\x10\x00\x01Not\xED"
                          "cias";
    ASSERT(dvb_get_string(out, sizeof(out), esc, sizeof(esc) - 1) == 0,
           "escaped latin1 should decode");
    ASSERT(strcmp(out, "Not\xC3\xAD"
                       "cias") == 0,
           "table escape should select 8859-1");
    // documents the #1477 default: prefix-less raw Latin-1 is read as
    // ISO-6937, whose E0-EF rows differ (0xED maps to U+0166)
    const uint8_t raw[] = "Not\xED"
                          "cias";
    ASSERT(dvb_get_string(out, sizeof(out), raw, sizeof(raw) - 1) == 0,
           "raw name should decode");
    ASSERT(strcmp(out, "Not\xC5\xA6"
                       "cias") == 0,
           "auto 6937 maps raw 0xED to U+0166 (#1477)");
    return 0;
}

int test_charset_override_ignores_prefix() {
    char out[64];
    // forced latin1 also decodes prefixed strings as Latin-1 (the 0x15
    // control byte is skipped, UTF-8 bytes become mojibake): documents
    // the override tradeoff, same as tvheadend
    const uint8_t utf8[] = "\x15Not\xC3\xAD"
                           "cias";
    dvb_set_charset_override(DVB_CHARSET_LATIN1);
    ASSERT(dvb_get_string(out, sizeof(out), utf8, sizeof(utf8) - 1) == 0,
           "forced latin1 should decode");
    ASSERT(strcmp(out, "Not\xC3\x83\xC2\xAD"
                       "cias") == 0,
           "forced latin1 reads UTF-8 bytes as Latin-1");
    dvb_set_charset_override(DVB_CHARSET_AUTO);
    return 0;
}

int main() {
    opts.log = 255;
    strcpy(thread_info[thread_index].thread_name, "test_dvb_charset");
    TEST_FUNC(test_charset_latin1_override(), "latin1 override failed");
    TEST_FUNC(test_charset_auto_keeps_prefix_detection(),
              "auto detection failed");
    TEST_FUNC(test_charset_override_ignores_prefix(),
              "prefix ignore failed");
    return 0;
}
