/*
Minisatip does not assume any responsability related to functionality of this code.
It also does not include any certificates, please procure them from alternative source 

 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>
#include <ctype.h>

#include <libdvben50221/en50221_session.h>
#include <libdvben50221/en50221_app_utils.h>
#include <libdvben50221/en50221_app_ai.h>
#include <libdvben50221/en50221_app_rm.h>
#include <libdvben50221/en50221_app_ca.h>
#include <libdvben50221/en50221_app_dvb.h>
#include <libdvben50221/en50221_app_datetime.h>
#include <libdvben50221/en50221_app_smartcard.h>
#include <libdvben50221/en50221_app_teletext.h>
#include <libdvben50221/en50221_app_tags.h>
#include <libdvben50221/en50221_app_mmi.h>
#include <libdvben50221/en50221_app_epg.h>
#include <libdvben50221/en50221_app_auth.h>
#include <libdvben50221/en50221_app_lowspeed.h>
#include <libdvbapi/dvbca.h>

#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

#include <linux/dvb/ca.h>
#include "dvb.h"
#include "socketworks.h"
#include "minisatip.h"
#include "dvbapi.h"
#include "adapter.h"
#include "search.h"
#include "ca.h"

#include "utils.h"

#define MAX_ELEMENTS 33
#define MAX_PAIRS 10
#define DEFAULT_LOG LOG_DVBCA
#define MAX_CA_PMT 32

#define TS101699_APP_RM_RESOURCEID MKRID(1, 1, 2)
#define TS101699_APP_AI_RESOURCEID MKRID(2, 1, 2)
#define CIPLUS_APP_AI_RESOURCEID MKRID(2, 1, 3)
#define CIPLUS_APP_AI_RESOURCEID_MULTI MKRID(2, 1, 4)
#define CIPLUS_APP_DVB_RESOURCEID MKRID(32, 1, 2)       //host control
#define CIPLUS_APP_DVB_RESOURCEID_TWO MKRID(32, 1, 3)   //host control
#define CIPLUS_APP_DVB_RESOURCEID_MULTI MKRID(32, 2, 1) //host control multistream
#define CIPLUS_APP_CA_RESOURCEID MKRID(3, 1, 1)
#define CIPLUS_APP_CA_RESOURCEID_MULTI MKRID(3, 2, 1)
//#define CIPLUS_APP_LOWSPEED_RESOURCEID	(DEVICE_TYPE, DEVICE_NUMBER) MKRID(96,((DEVICE_TYPE)<<2)|((DEVICE_NUMBER) & 0x03),2)
//#define CIPLUS_APP_LOWSPEED_RESOURCEID_TWO	(DEVICE_TYPE, DEVICE_NUMBER) MKRID(96,((DEVICE_TYPE)<<2)|((DEVICE_NUMBER) & 0x03),3) //CI+ v1.3
#define CIPLUS_APP_CC_RESOURCEID MKRID(140, 64, 1)
#define CIPLUS_APP_CC_RESOURCEID_TWO MKRID(140, 64, 2)   //CI+ v1.3
#define CIPLUS_APP_CC_RESOURCEID_THREE MKRID(140, 64, 3) //CI+ v1.3
#define CIPLUS_APP_CC_RESOURCEID_MULTI MKRID(140, 65, 1) //CI+ v1.3 multistream
#define CIPLUS_APP_LANG_RESOURCEID MKRID(141, 64, 1)
#define CIPLUS_APP_UPGR_RESOURCEID MKRID(142, 64, 1)
#define CIPLUS_APP_OPRF_RESOURCEID MKRID(143, 64, 1)
#define CIPLUS_APP_MULTISTREAM_RESOURCEID MKRID(144, 1, 1)
#define CIPLUS_APP_SAS_RESOURCEID MKRID(150, 64, 1)
#define TS101699_APP_AMMI_RESOURCEID MKRID(65, 1, 1)
#define CIPLUS_APP_AMMI_RESOURCEID MKRID(65, 1, 2)

#define CIPLUS_TAG_CC_OPEN_REQ 0x9f9001
#define CIPLUS_TAG_CC_OPEN_CNF 0x9f9002
#define CIPLUS_TAG_CC_DATA_REQ 0x9f9003
#define CIPLUS_TAG_CC_DATA_CNF 0x9f9004
#define CIPLUS_TAG_CC_SYNC_REQ 0x9f9005
#define CIPLUS_TAG_CC_SAC_DATA_REQ 0x9f9007
#define CIPLUS_TAG_CC_SAC_SYNC_REQ 0x9f9009
#define CIPLUS_TAG_APP_INFO 0x9f8021
#define CIPLUS_TAG_CICAM_RESET 0x9f8023
#define CIPLUS_TAG_COUNTRY_ENQ 0x9f8100
#define CIPLUS_TAG_LANG_ENQ 0x9f8110
#define CIPLUS_TAG_SAS_CONNECT_CNF 0x9f9a01
#define CIPLUS_TAG_FIRMWARE_UPGR 0x9f9d01
#define CIPLUS_TAG_FIRMWARE_UPGR_PRGRS 0x9f9d03
#define CIPLUS_TAG_FIRMWARE_UPGR_COMPLT 0x9f9d04
#define CIPLUS_TAG_OPERATOR_STATUS 0x9f9c01
#define CIPLUS_TAG_OPERATOR_INFO 0x9f9c05
#define CIPLUS_TAG_OPERATOR_SEARCH_STATUS 0x9f9c07
#define CIPLUS_TAG_OPERATOR_TUNE 0x9f9c09

#if OPENSSL_VERSION_NUMBER < 0x10100000L
int DH_set0_pqg(DH *dh, BIGNUM *p, BIGNUM *q, BIGNUM *g)
{
        /* If the fields p and g in d are NULL, the corresponding input
     * parameters MUST be non-NULL.  q may remain NULL.
     */
        if ((dh->p == NULL && p == NULL) || (dh->g == NULL && g == NULL))
                return 0;

        if (p != NULL)
        {
                BN_free(dh->p);
                dh->p = p;
        }
        if (q != NULL)
        {
                BN_free(dh->q);
                dh->q = q;
        }
        if (g != NULL)
        {
                BN_free(dh->g);
                dh->g = g;
        }

        if (q != NULL)
        {
                dh->length = BN_num_bits(q);
        }

        return 1;
}

void DH_get0_key(const DH *dh, const BIGNUM **pub_key, const BIGNUM **priv_key)
{
        if (pub_key != NULL)
                *pub_key = dh->pub_key;
        if (priv_key != NULL)
                *priv_key = dh->priv_key;
}

void DH_set_flags(DH *dh, int flags)
{
        dh->flags |= flags;
}
#endif

char ci_name_underscore[128];
int ci_number;
int logging = 0;
char logfile[256];
int extract_ci_cert = 0;

uint32_t datatype_sizes[MAX_ELEMENTS] = {
    0, 50, 0, 0, 0, 8, 8, 0,
    0, 0, 0, 0, 32, 256, 256, 0,
    0, 256, 256, 32, 8, 8, 32, 32,
    0, 8, 2, 32, 1, 32, 1, 0,
    32};

struct element
{
        uint8_t *data;
        uint32_t size;
        /* buffer valid */
        int valid;
};

struct ci_buffer
{
        size_t size;
        unsigned char data[];
};

struct ci_session
{
        /* parent */
        struct ca_device *ca;

        /* slot index */
        uint32_t slot_index;
        uint16_t index;
        uint32_t resid;
        int action;

        /* resources */
        const struct ci_resource *resource;

        /* private data */
        void *private_data;
};

typedef struct ca_device
{
        int enabled;
        int fd;
        int slot_id;
        int tc;
        int id;
        int ignore_close;
        int init_ok, pmt_id[MAX_CA_PMT];

        pthread_t stackthread;

        struct en50221_transport_layer *tl;
        struct en50221_session_layer *sl;

        struct en50221_app_send_functions sf;
        struct en50221_app_rm *rm_resource;
        struct en50221_app_ai *ai_resource;
        struct en50221_app_dvb *dvb_resource;
        struct en50221_app_ca *ca_resource;
        struct en50221_app_datetime *dt_resource;
        struct en50221_app_mmi *mmi_resource;

        int ca_high_bitrate_mode;
        int ca_ai_version;
        int ca_session_number;
        int uri_mask;

        uint16_t ai_session_number;
        uint16_t mmi_session_number;

        struct list_head *txq;
        struct list_head *mmiq;

        struct ci_session session[64];
        const struct ci_resource *resources[64];
        /*
   * CAM module info
   */
        char ci_name[128];

        char cam_menu_string[64];
        char pin_str[10];
        char force_ci;
        uint8_t key[2][16], iv[2][16];
        int sp, is_ciplus, parity;

} ca_device_t;

int dvbca_id;
static struct ca_device *ca_devices[MAX_ADAPTERS];

struct cc_ctrl_data
{

        /* parent */
        struct ci_session *session;

        /* ci+ credentials */
        struct element elements[MAX_ELEMENTS];

        /* DHSK */
        uint8_t dhsk[256];

        /* KS_host */
        uint8_t ks_host[32];

        /* derived keys */
        uint8_t sek[16];
        uint8_t sak[16];

        /* AKH checks - module performs 5 tries to get correct AKH */
        unsigned int akh_index;

        /* authentication data */
        uint8_t dh_exp[256];

        /* certificates */
        struct cert_ctx *cert_ctx;

        /* private key of device-cert */
        RSA *rsa_device_key;
};

struct cert_ctx
{
        X509_STORE *store;

        /* Host */
        X509 *cust_cert;
        X509 *device_cert;

        /* Module */
        X509 *ci_cust_cert;
        X509 *ci_device_cert;
};

struct aes_xcbc_mac_ctx
{
        uint8_t K[3][16];
        uint8_t IV[16];
        AES_KEY key;
        int buflen;
};

// this contains all known resource ids so we can see if the cam asks for something exotic
uint32_t resource_ids[] =
    {/* EN50221_APP_TELETEXT_RESOURCEID, EN50221_APP_SMARTCARD_RESOURCEID(1),EN50221_APP_AUTH_RESOURCEID,EN50221_APP_EPG_RESOURCEID(1) */ //not discribed in spec - so don't need
                                                                                                                                          //     EN50221_APP_RM_RESOURCEID,
     TS101699_APP_RM_RESOURCEID,
     //     EN50221_APP_AI_RESOURCEID,
     //     TS101699_APP_AI_RESOURCEID,
     CIPLUS_APP_AI_RESOURCEID,
     //     CIPLUS_APP_AI_RESOURCEID_MULTI,
     EN50221_APP_CA_RESOURCEID,
     //     CIPLUS_APP_CA_RESOURCEID_MULTI,
     //     EN50221_APP_DVB_RESOURCEID,
     CIPLUS_APP_DVB_RESOURCEID, //host control
                                //     CIPLUS_APP_DVB_RESOURCEID_TWO, //host control
                                //     CIPLUS_APP_DVB_RESOURCEID_MULTI, //host control
     EN50221_APP_DATETIME_RESOURCEID,
     EN50221_APP_MMI_RESOURCEID,
     EN50221_APP_LOWSPEED_RESOURCEID(1, 1), /* CIPLUS_APP_LOWSPEED_RESOURCEID(1, 1), CIPLUS_APP_LOWSPEED_RESOURCEID_TWO(1, 1), */
                                            //     CIPLUS_APP_CC_RESOURCEID,
     CIPLUS_APP_CC_RESOURCEID_TWO,
     //     CIPLUS_APP_CC_RESOURCEID_THREE,
     //     CIPLUS_APP_CC_RESOURCEID_MULTI,
     CIPLUS_APP_LANG_RESOURCEID,
     CIPLUS_APP_UPGR_RESOURCEID,
     CIPLUS_APP_OPRF_RESOURCEID,
     CIPLUS_APP_SAS_RESOURCEID,
     CIPLUS_APP_MULTISTREAM_RESOURCEID,
     //     TS101699_APP_AMMI_RESOURCEID,
     CIPLUS_APP_AMMI_RESOURCEID};

int resource_ids_count = sizeof(resource_ids) / 4;

uint32_t resource_ids_ci[] =
    {
        EN50221_APP_TELETEXT_RESOURCEID, EN50221_APP_SMARTCARD_RESOURCEID(1),
        EN50221_APP_RM_RESOURCEID, EN50221_APP_MMI_RESOURCEID,
        EN50221_APP_LOWSPEED_RESOURCEID(1, 1), EN50221_APP_EPG_RESOURCEID(1),
        EN50221_APP_DVB_RESOURCEID, EN50221_APP_CA_RESOURCEID,
        EN50221_APP_DATETIME_RESOURCEID, EN50221_APP_AUTH_RESOURCEID,
        EN50221_APP_AI_RESOURCEID, TS101699_APP_AI_RESOURCEID, CIPLUS_APP_AI_RESOURCEID};

int resource_ids_ci_count = sizeof(resource_ids_ci) / 4;

typedef enum
{
        CIPLUS_DATA_RATE_72_MBPS = 0,
        CIPLUS_DATA_RATE_96_MBPS = 1,
} ciplus13_data_rate_t;

unsigned char dh_p[256] = {/* prime */
                           0xd6, 0x27, 0x14, 0x7a, 0x7c, 0x0c, 0x26, 0x63, 0x9d, 0x82, 0xeb, 0x1f, 0x4a, 0x18, 0xff, 0x6c,
                           0x34, 0xad, 0xea, 0xa6, 0xc0, 0x23, 0xe6, 0x65, 0xfc, 0x8e, 0x32, 0xc3, 0x33, 0xf4, 0x91, 0xa7,
                           0xcc, 0x88, 0x58, 0xd7, 0xf3, 0xb3, 0x17, 0x5e, 0xb0, 0xa8, 0xeb, 0x5c, 0xd4, 0xd8, 0x3a, 0xae,
                           0x8e, 0x75, 0xa1, 0x50, 0x5f, 0x5d, 0x67, 0xc5, 0x40, 0xf4, 0xb3, 0x68, 0x35, 0xd1, 0x3a, 0x4c,
                           0x93, 0x7f, 0xca, 0xce, 0xdd, 0x83, 0x29, 0x01, 0xc8, 0x4b, 0x76, 0x81, 0x56, 0x34, 0x83, 0x31,
                           0x92, 0x72, 0x65, 0x7b, 0xac, 0xd9, 0xda, 0xa9, 0xd1, 0xd3, 0xe5, 0x77, 0x58, 0x6f, 0x5b, 0x44,
                           0x3e, 0xaf, 0x7f, 0x6d, 0xf5, 0xcf, 0x0a, 0x80, 0x0d, 0xa5, 0x56, 0x4f, 0x4b, 0x85, 0x41, 0x0f,
                           0x13, 0x41, 0x06, 0x1f, 0xf3, 0xd9, 0x65, 0x36, 0xae, 0x47, 0x41, 0x1f, 0x1f, 0xe0, 0xde, 0x69,
                           0xe5, 0x86, 0x2a, 0xa1, 0xf2, 0x48, 0x02, 0x92, 0x68, 0xa6, 0x37, 0x9f, 0x76, 0x4f, 0x7d, 0x94,
                           0x5d, 0x10, 0xe5, 0xab, 0x5d, 0xb2, 0xf3, 0x12, 0x8c, 0x79, 0x03, 0x92, 0xa6, 0x7f, 0x8a, 0x78,
                           0xb0, 0xba, 0xc5, 0xb5, 0x31, 0xc5, 0xc8, 0x22, 0x6e, 0x29, 0x02, 0x40, 0xab, 0xe7, 0x5c, 0x23,
                           0x33, 0x7f, 0xcb, 0x86, 0xc7, 0xb4, 0xfd, 0xaa, 0x44, 0xcd, 0x9c, 0x9f, 0xba, 0xac, 0x3a, 0xcf,
                           0x7e, 0x31, 0x5f, 0xa8, 0x47, 0xce, 0xca, 0x1c, 0xb4, 0x77, 0xa0, 0xec, 0x9a, 0x46, 0xd4, 0x79,
                           0x7b, 0x64, 0xbb, 0x6c, 0x91, 0xb2, 0x38, 0x01, 0x65, 0x11, 0x45, 0x9f, 0x62, 0x08, 0x6f, 0x31,
                           0xcf, 0xc4, 0xba, 0xdc, 0xd0, 0x03, 0x91, 0xf1, 0x18, 0x1f, 0xcb, 0x4d, 0xfc, 0x73, 0x5a, 0xa2,
                           0x15, 0xb8, 0x3c, 0x8d, 0x80, 0x92, 0x1c, 0xa1, 0x03, 0xd0, 0x83, 0x2f, 0x5f, 0xe3, 0x07, 0x69};

unsigned char dh_g[256] = {/* generator */
                           0x95, 0x7d, 0xd1, 0x49, 0x68, 0xc1, 0xa5, 0xf1, 0x48, 0xe6, 0x50, 0x4f, 0xa1, 0x10, 0x72, 0xc4,
                           0xef, 0x12, 0xec, 0x2d, 0x94, 0xbe, 0xc7, 0x20, 0x2c, 0x94, 0xf9, 0x68, 0x67, 0x0e, 0x22, 0x17,
                           0xb5, 0x5c, 0x0b, 0xca, 0xac, 0x9f, 0x25, 0x9c, 0xd2, 0xa6, 0x1a, 0x20, 0x10, 0x16, 0x6a, 0x42,
                           0x27, 0x83, 0x47, 0x42, 0xa0, 0x07, 0x52, 0x09, 0x33, 0x97, 0x4e, 0x30, 0x57, 0xd8, 0xb7, 0x1e,
                           0x46, 0xa6, 0xba, 0x4e, 0x40, 0x6a, 0xe9, 0x1a, 0x5a, 0xa0, 0x74, 0x56, 0x92, 0x55, 0xc2, 0xbd,
                           0x44, 0xcd, 0xb3, 0x33, 0xf7, 0x35, 0x46, 0x25, 0xdf, 0x84, 0x19, 0xf3, 0xe2, 0x7a, 0xac, 0x4e,
                           0xee, 0x1a, 0x86, 0x3b, 0xb3, 0x87, 0xa6, 0x66, 0xc1, 0x70, 0x21, 0x41, 0xd3, 0x58, 0x36, 0xb5,
                           0x3b, 0x6e, 0xa1, 0x55, 0x60, 0x9a, 0x59, 0xd3, 0x85, 0xd8, 0xdc, 0x6a, 0xff, 0x41, 0xb6, 0xbf,
                           0x42, 0xde, 0x64, 0x00, 0xd0, 0xee, 0x3a, 0xa1, 0x8a, 0xed, 0x12, 0xf9, 0xba, 0x54, 0x5c, 0xdb,
                           0x06, 0x24, 0x49, 0xe8, 0x47, 0xcf, 0x5b, 0xe4, 0xbb, 0xc0, 0xaa, 0x8a, 0x8c, 0xbe, 0x73, 0xd9,
                           0x02, 0xea, 0xee, 0x8d, 0x87, 0x5b, 0xbf, 0x78, 0x04, 0x41, 0x9e, 0xa8, 0x5c, 0x3c, 0x49, 0xde,
                           0x88, 0x6d, 0x62, 0x21, 0x7f, 0xf0, 0x5e, 0x2d, 0x1d, 0xfc, 0x47, 0x0d, 0x1b, 0xaa, 0x4e, 0x0d,
                           0x78, 0x20, 0xfe, 0x57, 0x0f, 0xca, 0xdf, 0xeb, 0x3c, 0x84, 0xa7, 0xe1, 0x61, 0xb2, 0x95, 0x98,
                           0x07, 0x73, 0x8e, 0x51, 0xc6, 0x87, 0xe4, 0xcf, 0xf1, 0x5f, 0x86, 0x99, 0xec, 0x8d, 0x44, 0x92,
                           0x2c, 0x99, 0xf6, 0xc0, 0xf4, 0x39, 0xe8, 0x05, 0xbf, 0xc1, 0x56, 0xde, 0xfe, 0x93, 0x75, 0x06,
                           0x69, 0x87, 0x83, 0x06, 0x51, 0x80, 0xa5, 0x6e, 0xa6, 0x19, 0x7d, 0x3b, 0xef, 0xfb, 0xe0, 0x4a};

int dvbca_close_device(ca_device_t *c);

////// MISC.C

int get_random(unsigned char *dest, int len)
{
        int fd;
        char *urnd = "/dev/urandom";

        fd = open(urnd, O_RDONLY);
        if (fd < 0)
        {
                LOG("cannot open %s", urnd);
                return -1;
        }

        if (read(fd, dest, len) != len)
        {
                LOG("cannot read from %s", urnd);
                close(fd);
                return -2;
        }

        close(fd);

        return len;
}

int parseLengthField(const unsigned char *pkt, int *len)
{
        int i;

        *len = 0;
        if (!(*pkt & 0x80))
        {
                *len = *pkt;
                return 1;
        }
        for (i = 0; i < (pkt[0] & 0x7F); ++i)
        {
                *len <<= 8;
                *len |= pkt[i + 1];
        }
        return (pkt[0] & 0x7F) + 1;
}

int add_padding(uint8_t *dest, unsigned int len, unsigned int blocklen)
{
        uint8_t padding = 0x80;
        int count = 0;

        while (len & (blocklen - 1))
        {
                *dest++ = padding;
                ++len;
                ++count;
                padding = 0;
        }

        return count;
}

static int get_bin_from_nibble(int in)
{
        if ((in >= '0') && (in <= '9'))
                return in - 0x30;

        if ((in >= 'A') && (in <= 'Z'))
                return in - 0x41 + 10;

        if ((in >= 'a') && (in <= 'z'))
                return in - 0x61 + 10;

        LOG("fixme: unsupported chars in hostid");

        return 0;
}

void str2bin(uint8_t *dst, char *data, int len)
{
        int i;

        for (i = 0; i < len; i += 2)
                *dst++ = (get_bin_from_nibble(data[i]) << 4) | get_bin_from_nibble(data[i + 1]);
}

uint32_t UINT32(const unsigned char *in, unsigned int len)
{
        uint32_t val = 0;
        unsigned int i;

        for (i = 0; i < len; i++)
        {
                val <<= 8;
                val |= *in++;
        }

        return val;
}

int BYTE32(unsigned char *dest, uint32_t val)
{
        *dest++ = val >> 24;
        *dest++ = val >> 16;
        *dest++ = val >> 8;
        *dest++ = val;

        return 4;
}

int BYTE16(unsigned char *dest, uint16_t val)
{
        *dest++ = val >> 8;
        *dest++ = val;
        return 2;
}

void cert_strings(char *certfile)
{
        int c;
        unsigned count;
        //      off_t offset;
        FILE *file;
        char string[256];
        int n = 2; /* too short string to be usefull */
        int line = 0;

        file = fopen(certfile, "r");
        if (!file)
        {
                LOG("Could not open certificate file %s", certfile);
                return;
        }
        LOG("#########################################################\n");
        //      offset = 0;
        count = 0;
        do
        {
                if (line > 14)
                        n = 8; /* after usefull info be stricter */
                c = fgetc(file);
                //              if (isprint(c) || c == '\t')
                if (isprint(c))
                {
                        string[count] = c;
                        count++;
                }
                else
                {
                        if (count > n) /* line feed */
                        {
                                string[count - 1] = 0;
                                LOG("%s\n", string);
                                line++;
                        }
                        count = 0;
                }
                //              offset++;
        } while ((c != EOF) && (line < 16)); /* only frst 15 lines */
        fclose(file);
        LOG("#########################################################\n");
        return;
}

/// END MISC

///// AES_XCBC_MAC

int aes_xcbc_mac_init(struct aes_xcbc_mac_ctx *ctx, const uint8_t *key)
{
        AES_KEY aes_key;
        int y, x;

        AES_set_encrypt_key(key, 128, &aes_key);

        for (y = 0; y < 3; y++)
        {
                for (x = 0; x < 16; x++)
                        ctx->K[y][x] = y + 1;
                AES_ecb_encrypt(ctx->K[y], ctx->K[y], &aes_key, 1);
        }

        /* setup K1 */
        AES_set_encrypt_key(ctx->K[0], 128, &ctx->key);

        memset(ctx->IV, 0, 16);
        ctx->buflen = 0;

        return 0;
}

int aes_xcbc_mac_process(struct aes_xcbc_mac_ctx *ctx, const uint8_t *in, unsigned int len)
{
        while (len)
        {
                if (ctx->buflen == 16)
                {
                        AES_ecb_encrypt(ctx->IV, ctx->IV, &ctx->key, 1);
                        ctx->buflen = 0;
                }
                ctx->IV[ctx->buflen++] ^= *in++;
                --len;
        }

        return 0;
}

int aes_xcbc_mac_done(struct aes_xcbc_mac_ctx *ctx, uint8_t *out)
{
        int i;

        if (ctx->buflen == 16)
        {
                /* K2 */
                for (i = 0; i < 16; i++)
                        ctx->IV[i] ^= ctx->K[1][i];
        }
        else
        {
                ctx->IV[ctx->buflen] ^= 0x80;
                /* K3 */
                for (i = 0; i < 16; i++)
                        ctx->IV[i] ^= ctx->K[2][i];
        }

        AES_ecb_encrypt(ctx->IV, ctx->IV, &ctx->key, 1);
        memcpy(out, ctx->IV, 16);

        return 0;
}

////// END_AES_XCBC_MAC
////// DH_RSA_MISC

static int pkcs_1_mgf1(const uint8_t *seed, unsigned long seedlen, uint8_t *mask, unsigned long masklen)
{
        unsigned long hLen, x;
        uint32_t counter;
        uint8_t *buf;

        /* get hash output size */
        hLen = 20; /* SHA1 */

        /* allocate memory */
        buf = malloc(hLen);
        if (buf == NULL)
        {
                LOG("error mem");
                return -1;
        }

        /* start counter */
        counter = 0;

        while (masklen > 0)
        {
                /* handle counter */
                BYTE32(buf, counter);
                ++counter;

                /* get hash of seed || counter */
                unsigned char buffer[0x18];
                memcpy(buffer, seed, seedlen);
                memcpy(buffer + 0x14, buf, 4);
                SHA1(buffer, 0x18, buf);

                /* store it */
                for (x = 0; x < hLen && masklen > 0; x++, masklen--)
                        *mask++ = buf[x];
        }

        free(buf);
        return 0;
}

static int pkcs_1_pss_encode(const uint8_t *msghash, unsigned int msghashlen,
                             unsigned long saltlen, unsigned long modulus_bitlen,
                             uint8_t *out, unsigned int outlen)
{
        unsigned char *DB, *mask, *salt, *hash;
        unsigned long x, y, hLen, modulus_len;
        int err = -1;
        unsigned char *hashbuf;
        unsigned int hashbuflen;

        hLen = 20; /* SHA1 */
        modulus_len = (modulus_bitlen >> 3) + (modulus_bitlen & 7 ? 1 : 0);

        /* allocate ram for DB/mask/salt/hash of size modulus_len */
        DB = malloc(modulus_len);
        mask = malloc(modulus_len);
        salt = malloc(modulus_len);
        hash = malloc(modulus_len);

        hashbuflen = 8 + msghashlen + saltlen;
        hashbuf = malloc(hashbuflen);

        if (!(DB && mask && salt && hash && hashbuf))
        {
                LOG("out of memory");
                goto LBL_ERR;
        }

        /* generate random salt */
        if (saltlen > 0)
        {
                if (get_random(salt, saltlen) != (long)saltlen)
                {
                        LOG("rnd failed");
                        goto LBL_ERR;
                }
        }

        /* M = (eight) 0x00 || msghash || salt, hash = H(M) */
        memset(hashbuf, 0, 8);
        memcpy(hashbuf + 8, msghash, msghashlen);
        memcpy(hashbuf + 8 + msghashlen, salt, saltlen);
        SHA1(hashbuf, hashbuflen, hash);

        /* generate DB = PS || 0x01 || salt, PS == modulus_len - saltlen - hLen - 2 zero bytes */
        x = 0;
        memset(DB + x, 0, modulus_len - saltlen - hLen - 2);
        x += modulus_len - saltlen - hLen - 2;
        DB[x++] = 0x01;
        memcpy(DB + x, salt, saltlen);
        x += saltlen;

        err = pkcs_1_mgf1(hash, hLen, mask, modulus_len - hLen - 1);
        if (err)
                goto LBL_ERR;

        /* xor against DB */
        for (y = 0; y < (modulus_len - hLen - 1); y++)
                DB[y] ^= mask[y];

        /* output is DB || hash || 0xBC */
        if (outlen < modulus_len)
        {
                err = -1;
                LOG("error overflow");
                goto LBL_ERR;
        }

        /* DB len = modulus_len - hLen - 1 */
        y = 0;
        memcpy(out + y, DB, modulus_len - hLen - 1);
        y += modulus_len - hLen - 1;

        /* hash */
        memcpy(out + y, hash, hLen);
        y += hLen;

        /* 0xBC */
        out[y] = 0xBC;

        /* now clear the 8*modulus_len - modulus_bitlen most significant bits */
        out[0] &= 0xFF >> ((modulus_len << 3) - (modulus_bitlen - 1));

        err = 0;
LBL_ERR:
        free(hashbuf);
        free(hash);
        free(salt);
        free(mask);
        free(DB);

        return err;
}

/* DH */

int dh_gen_exp(uint8_t *dest, int dest_len, uint8_t *dh_g, int dh_g_len, uint8_t *dh_p, int dh_p_len)
{
        DH *dh;
        BIGNUM *p, *g;
        const BIGNUM *priv_key;
        int len;
        unsigned int gap;

        dh = DH_new();

        p = BN_bin2bn(dh_p, dh_p_len, 0);
        g = BN_bin2bn(dh_g, dh_g_len, 0);
        DH_set0_pqg(dh, p, NULL, g);
        DH_set_flags(dh, DH_FLAG_NO_EXP_CONSTTIME);

        DH_generate_key(dh);

        DH_get0_key(dh, NULL, &priv_key);
        len = BN_num_bytes(priv_key);
        if (len > dest_len)
        {
                LOG("len > dest_len");
                return -1;
        }

        gap = dest_len - len;
        memset(dest, 0, gap);
        BN_bn2bin(priv_key, &dest[gap]);

        DH_free(dh);

        return 0;
}

/* dest = base ^ exp % mod */
int dh_mod_exp(uint8_t *dest, int dest_len, uint8_t *base, int base_len, uint8_t *mod, int mod_len, uint8_t *exp, int exp_len)
{
        BIGNUM *bn_dest, *bn_base, *bn_exp, *bn_mod;
        BN_CTX *ctx;
        int len;
        unsigned int gap;

        bn_base = BN_bin2bn(base, base_len, NULL);
        bn_exp = BN_bin2bn(exp, exp_len, NULL);
        bn_mod = BN_bin2bn(mod, mod_len, NULL);
        ctx = BN_CTX_new();

        bn_dest = BN_new();
        BN_mod_exp(bn_dest, bn_base, bn_exp, bn_mod, ctx);
        BN_CTX_free(ctx);

        len = BN_num_bytes(bn_dest);
        if (len > dest_len)
        {
                LOG("len > dest_len");
                return -1;
        }

        gap = dest_len - len;
        memset(dest, 0, gap);
        BN_bn2bin(bn_dest, &dest[gap]);

        BN_free(bn_dest);
        BN_free(bn_mod);
        BN_free(bn_exp);
        BN_free(bn_base);

        return 0;
}

int dh_dhph_signature(uint8_t *out, uint8_t *nonce, uint8_t *dhph, RSA *r)
{
        unsigned char dest[302];
        uint8_t hash[20];
        unsigned char dbuf[256];

        dest[0x00] = 0x00; /* version */
        dest[0x01] = 0x00;
        dest[0x02] = 0x08; /* len (bits) */
        dest[0x03] = 0x01; /* version data */

        dest[0x04] = 0x01; /* msg_label */
        dest[0x05] = 0x00;
        dest[0x06] = 0x08; /* len (bits) */
        dest[0x07] = 0x02; /* message data */

        dest[0x08] = 0x02; /* auth_nonce */
        dest[0x09] = 0x01;
        dest[0x0a] = 0x00; /* len (bits) */
        memcpy(&dest[0x0b], nonce, 32);

        dest[0x2b] = 0x04; /* DHPH - DH public key host */
        dest[0x2c] = 0x08;
        dest[0x2d] = 0x00; /* len (bits) */
        memcpy(&dest[0x2e], dhph, 256);

        SHA1(dest, 0x12e, hash);

        if (pkcs_1_pss_encode(hash, 20, 20, 0x800, dbuf, sizeof(dbuf)))
        {
                LOG("pss encode failed");
                return -1;
        }

        RSA_private_encrypt(sizeof(dbuf), dbuf, out, r, RSA_NO_PADDING);

        return 0;
}

int is_ca_initialized(int i)
{
        if (i >= 0 && i < MAX_CA && ca_devices[i] && ca_devices[i]->enabled && ca_devices[i]->init_ok)
                return 1;
        return 0;
}

int createCAPMT(uint8_t *b, int len, int listmgmt, uint8_t *capmt, int capmt_len, int reason)
{
        struct section *section = section_codec(b, len);
        int size;
        if (!section)
        {
                LOG("failed to decode section");
                return 0;
        }

        struct section_ext *result = section_ext_decode(section, 0);
        if (!result)
        {
                LOG("failed to decode ext_section");
                return 0;
        }

        struct mpeg_pmt_section *pmt = mpeg_pmt_section_codec(result);
        if (!pmt)
        {
                LOG("failed to decode pmt");
                return 0;
        }

        if ((size = en50221_ca_format_pmt((struct mpeg_pmt_section *)b, capmt, capmt_len, 0, listmgmt,
                                          reason)) < 0)
                LOG("Failed to format CA PMT object");
        return size;
}

int sendCAPMT(struct en50221_app_ca *ca_resource, int ca_session_number, uint8_t *b, int len, int listmgmt, int reason)
{
        uint8_t capmt[8192];
        uint8_t init_b[len + 10];
        int rc;
        memset(init_b, 0, sizeof(init_b));
        memcpy(init_b, b, len);
        int size = createCAPMT(init_b, len, listmgmt, capmt, sizeof(capmt), reason);

        if (size <= 0)
                LOG_AND_RETURN(TABLES_RESULT_ERROR_NORETRY, "createCAPMT failed");

        if ((rc = en50221_app_ca_pmt(ca_resource, ca_session_number, capmt, size)))
        {
                LOG("Failed to send CA PMT object, error %d", rc);
        }
        return rc;
}

int dvbca_process_pmt(adapter *ad, SPMT *spmt)
{
        ca_device_t *d = ca_devices[ad->id];
        uint16_t pid, sid, ver;
        int len, listmgmt, i, ca_pos;
        uint8_t *b = spmt->pmt;

        if (!d)
                return TABLES_RESULT_ERROR_NORETRY;
        if (!d->init_ok)
                LOG_AND_RETURN(TABLES_RESULT_ERROR_RETRY, "CAM not yet initialized");

        pid = spmt->pid;
        len = spmt->pmt_len;
        for (ca_pos = 0; ca_pos < MAX_CA_PMT; ca_pos++)
                if ((d->pmt_id[ca_pos] == -1) || (d->pmt_id[ca_pos] == spmt->id))
                        break;

        if (ca_pos < MAX_CA_PMT)
                d->pmt_id[ca_pos] = spmt->id;
        else
                LOG_AND_RETURN(TABLES_RESULT_ERROR_RETRY, "pmt_id full for device %d", d->id);

        ver = spmt->version;
        sid = spmt->sid;

        listmgmt = CA_LIST_MANAGEMENT_ONLY;
        for (i = 0; i < MAX_CA_PMT; i++)
                if (d->pmt_id[i] > 0 && d->pmt_id[i] != spmt->id)
                {
                        listmgmt = CA_LIST_MANAGEMENT_ADD;
                }

        LOG("PMT CA %d pid %u (%s) len %u ver %u sid %u (%x), pos %d, %s", spmt->adapter, pid, spmt->name, len, ver, sid, sid,
            ca_pos, listmgmt == CA_LIST_MANAGEMENT_ONLY ? "only" : "add");

        if (sendCAPMT(d->ca_resource, d->ca_session_number, b, len, listmgmt, CA_PMT_CMD_ID_OK_DESCRAMBLING))
                LOG_AND_RETURN(TABLES_RESULT_ERROR_NORETRY, "sendCAPMT failed");

        if (d->key[0][0])
                send_cw(spmt->id, CA_ALGO_AES128_CBC, 0, d->key[0], d->iv[0], 3600); // 1 hour
        if (d->key[1][0])
                send_cw(spmt->id, CA_ALGO_AES128_CBC, 1, d->key[1], d->iv[1], 3600);

        return 0;
}

int dvbca_del_pmt(adapter *ad, SPMT *spmt)
{
        ca_device_t *d = ca_devices[ad->id];
        int i, ca_pos = -1;
        int num_pmt = 0;
        for (i = 0; i < MAX_CA_PMT; i++)
                if (d->pmt_id[i] == spmt->id)
                {
                        ca_pos = i;
                        d->pmt_id[i] = -1;
                }
                else if (d->pmt_id[i] > 0)
                        num_pmt++;

        LOG("PMT CA %d DEL pid %u (%s) sid %u (%x), ver %d, pos %d, num pmt %d, %s",
            spmt->adapter, spmt->pid, spmt->name, spmt->sid, spmt->sid, spmt->version, ca_pos, num_pmt, spmt->name);
        uint8_t clean[1500];
        int new_len = clean_psi_buffer(spmt->pmt, clean, sizeof(clean));

        if (new_len < 1)
                LOG_AND_RETURN(0, "Could not clean the PSI information for PMT %d", spmt->id);

        if (sendCAPMT(d->ca_resource, d->ca_session_number, spmt->pmt, spmt->pmt_len, CA_LIST_MANAGEMENT_UPDATE, CA_PMT_CMD_ID_NOT_SELECTED))
                LOG_AND_RETURN(TABLES_RESULT_ERROR_NORETRY, "%s: sendCAPMT for clean PMT failed", __FUNCTION__);

        return 0;
}

static int ciplus13_app_ai_data_rate_info(ca_device_t *d, ciplus13_data_rate_t rate)
{
        uint8_t data[] = {0x9f, 0x80, 0x24, 0x01, (uint8_t)rate};

        /* only version 3 (CI+ 1.3) supports data_rate_info -  no it isn't, 1.2 support too*/
        if (d->ca_ai_version < 2)
                return 0;

        LOG("setting CI+ CAM data rate to %s Mbps", rate ? "96" : "72");

        return en50221_sl_send_data(d->sl, d->ai_session_number, data, sizeof(data));
}

int ca_ai_callback(void *arg, uint8_t slot_id, uint16_t session_number,
                   uint8_t application_type, uint16_t application_manufacturer,
                   uint16_t manufacturer_code, uint8_t menu_string_length,
                   uint8_t *menu_string)
{
        ca_device_t *d = arg;

        LOG("%02x:%s", slot_id, __func__);
        LOG("  Application type: %02x", application_type);
        LOG("  Application manufacturer: %04x", application_manufacturer);
        LOG("  Manufacturer code: %04x", manufacturer_code);
        LOG("  Menu string: %.*s", menu_string_length, menu_string);

        d->ai_session_number = session_number;
        memcpy(d->ci_name, menu_string, menu_string_length);
        return 0;
}

static struct element *element_get(struct cc_ctrl_data *cc_data, unsigned int id)
{
        /* array index */
        if ((id < 1) || (id >= MAX_ELEMENTS))
        {
                LOG("element_get: invalid id");
                return NULL;
        }
        return &cc_data->elements[id];
}

static void element_invalidate(struct cc_ctrl_data *cc_data, unsigned int id)
{
        struct element *e;

        e = element_get(cc_data, id);
        if (e)
        {
                free(e->data);
                memset(e, 0, sizeof(struct element));
        }
}

static void element_init(struct cc_ctrl_data *cc_data)
{
        unsigned int i;

        for (i = 1; i < MAX_ELEMENTS; i++)
                element_invalidate(cc_data, i);
}

static int element_set(struct cc_ctrl_data *cc_data, unsigned int id, const uint8_t *data, uint32_t size)
{
        struct element *e;

        e = element_get(cc_data, id);
        if (e == NULL)
                return 0;

        /* check size */
        if ((datatype_sizes[id] != 0) && (datatype_sizes[id] != size))
        {
                LOG("size %d of datatype_id %d doesn't match", size, id);
                return 0;
        }

        free(e->data);
        e->data = malloc(size);
        memcpy(e->data, data, size);
        e->size = size;
        e->valid = 1;

        LOG("_element_set_ stored %d with len %d", id, size);
        //       hexdump("DATA: ", (void *)data, size);
        return 1;
}

static int element_set_certificate(struct cc_ctrl_data *cc_data, unsigned int id, X509 *cert)
{
        unsigned char *cert_der = NULL;
        int cert_len;

        cert_len = i2d_X509(cert, &cert_der);
        if (cert_len <= 0)
        {
                LOG("can not get data in DER format");
                return 0;
        }

        if (!element_set(cc_data, id, cert_der, cert_len))
        {
                LOG("can not store element (%d)", id);
                return 0;
        }

        return 1;
}

static int element_set_hostid_from_certificate(struct cc_ctrl_data *cc_data, unsigned int id, X509 *cert)
{
        X509_NAME *subject;
        int nid_cn = OBJ_txt2nid("CN");
        char hostid[20];
        uint8_t bin_hostid[8];

        if ((id != 5) && (id != 6))
        {
                LOG("wrong datatype_id for hostid");
                return 0;
        }

        subject = X509_get_subject_name(cert);
        X509_NAME_get_text_by_NID(subject, nid_cn, hostid, sizeof(hostid));

        if (strlen(hostid) != 16)
        {
                LOG("malformed hostid");
                return 0;
        }
        LOG("%sID: %s", id == 5 ? "HOST" : "CICAM", hostid);

        str2bin(bin_hostid, hostid, 16);

        if (!element_set(cc_data, id, bin_hostid, sizeof(bin_hostid)))
        {
                LOG("can not set hostid");
                return 0;
        }

        return 1;
}

static int element_valid(struct cc_ctrl_data *cc_data, unsigned int id)
{
        struct element *e;

        e = element_get(cc_data, id);

        return e && e->valid;
}

static unsigned int element_get_buf(struct cc_ctrl_data *cc_data, uint8_t *dest, unsigned int id)
{
        struct element *e;

        e = element_get(cc_data, id);
        if (e == NULL)
                return 0;

        if (!e->valid)
        {
                LOG("element_get_buf: datatype %d not valid", id);
                return 0;
        }

        if (!e->data)
        {
                LOG("element_get_buf: datatype %d doesn't exist", id);
                return 0;
        }

        if (dest)
                memcpy(dest, e->data, e->size);

        return e->size;
}

static unsigned int element_get_req(struct cc_ctrl_data *cc_data, uint8_t *dest, unsigned int id)
{
        unsigned int len = element_get_buf(cc_data, &dest[3], id);

        if (len == 0)
        {
                LOG("can not get element %d", id);
                return 0;
        }

        dest[0] = id;
        dest[1] = len >> 8;
        dest[2] = len;

        return 3 + len;
}

static uint8_t *element_get_ptr(struct cc_ctrl_data *cc_data, unsigned int id)
{
        struct element *e;

        e = element_get(cc_data, id);
        if (e == NULL)
                return NULL;

        if (!e->valid)
        {
                LOG("element_get_ptr: datatype %u not valid", id);
                return NULL;
        }

        if (!e->data)
        {
                LOG("element_get_ptr: datatype %u doesn't exist", id);

                return NULL;
        }

        return e->data;
}

static void get_authdata_filename(char *dest, size_t len, unsigned int slot, char *ci_name)
{
        char cin[128];
        char source[256];
        char target[256];
        /* add module name to slot authorization bin file */
        memset(cin, 0, sizeof(cin));
        strncpy(cin, ci_name, sizeof(cin) - 1);
        FILE *auth_bin;
        /* quickly replace blanks */
        int i = 0;
        while (cin[i] != 0)
        {
                if (cin[i] == 32)
                        cin[i] = 95; /* underscore _ */
                i++;
        };
        snprintf(source, sizeof(source) - 1, "%s/ci_auth_%d.bin", getenv("HOME"), slot);
        snprintf(target, sizeof(target) - 1, "%s/ci_auth_%s_%d.bin", getenv("HOME"), cin, slot);

        struct stat buf;

        if (lstat(source, &buf) == 0)
        {

                char linkname[4096];
                memset(linkname, 0, sizeof(linkname));
                ssize_t len = readlink(source, linkname, sizeof(linkname) - 1);
                if (len > 0)
                {
                        if (strcmp(linkname, target) != 0)
                        {
                                /* link doesn't point to target */
                                auth_bin = fopen(target, "r");
                                if (auth_bin)
                                        fclose(auth_bin);
                                /* correct symlink */
                                int r = remove(source);
                                LOG("CORRECTING %s to %s %s", target, source, r ? "" : "(remove failed)");
                                symlink(target, source);
                        }
                }
                else
                {
                        auth_bin = fopen(target, "r");
                        if (auth_bin)
                        {
                                /* if new file already exists and source is not symlink.
                      remove and do symlink */
                                fclose(auth_bin);
                                int r = remove(source);
                                LOG("LINKING %s to %s %s", target, source, r ? "" : "(remove failed)");
                                symlink(target, source);
                        }
                        else
                        {
                                /* target doesn't exist needs migration...
                      which means rename old bin file without module name
                      to new file with module name and do symlink to old one */
                                LOG("MIGRATING %s to %s", source, target);
                                if (!rename(source, target))
                                        symlink(target, source);
                        }
                }
        }
        else
        {
                auth_bin = fopen(target, "r");
                if (auth_bin)
                {
                        /* if new file already exists and source is nit there
                      simply do symlink */
                        fclose(auth_bin);
                        LOG("LINKING %s to %s", target, source);
                        symlink(target, source);
                }
                /* else do nothing to prevent stranded symlinks */
        }
        snprintf(dest, len, "%s/ci_auth_%s_%d.bin", getenv("HOME"), cin, slot);
}

static int get_authdata(uint8_t *host_id, uint8_t *dhsk, uint8_t *akh, unsigned int slot, unsigned int index, char *ci_name)
{
        char filename[FILENAME_MAX];
        int fd;
        uint8_t chunk[8 + 256 + 32];
        unsigned int i;
        /* 5 pairs of data only */
        if (index > MAX_PAIRS)
                return 0;

        get_authdata_filename(filename, sizeof(filename), slot, ci_name);

        fd = open(filename, O_RDONLY);
        if (fd < 0)
        {
                LOG("cannot open %s", filename);
                return 0;
        }

        for (i = 0; i < MAX_PAIRS; i++)
        {
                if (read(fd, chunk, sizeof(chunk)) != sizeof(chunk))
                {
                        LOG("cannot read auth_data");
                        close(fd);
                        return 0;
                }
                if (i == index)
                {
                        memcpy(host_id, chunk, 8);
                        memcpy(dhsk, &chunk[8], 256);
                        memcpy(akh, &chunk[8 + 256], 32);
                        close(fd);
                        return 1;
                }
        }

        close(fd);
        return 0;
}

static int write_authdata(unsigned int slot, const uint8_t *host_id, const uint8_t *dhsk, const uint8_t *akh, char *ci_name)
{
        char filename[FILENAME_MAX];
        int fd;
        uint8_t buf[(8 + 256 + 32) * MAX_PAIRS];
        unsigned int entries;
        unsigned int i;

        int ret = 0;

        for (entries = 0; entries < MAX_PAIRS; entries++)
        {
                int offset = (8 + 256 + 32) * entries;
                if (!get_authdata(&buf[offset], &buf[offset + 8], &buf[offset + 8 + 256], slot, entries, ci_name))
                        break;

                /* check if we got this pair already */
                if (!memcmp(&buf[offset + 8 + 256], akh, 32))
                {
                        LOG("data already stored");
                        return 1;
                }
        }

        LOG("got %d entries", entries);

        get_authdata_filename(filename, sizeof(filename), slot, ci_name);

        fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd < 0)
        {
                LOG("cannot open %s for writing - authdata not stored", filename);
                return 0;
        }

        /* store new entry first */
        if (write(fd, host_id, 8) != 8)
        {
                LOG("error in write");
                goto end;
        }

        if (write(fd, dhsk, 256) != 256)
        {
                LOG("error in write");
                goto end;
        }

        if (write(fd, akh, 32) != 32)
        {
                LOG("error in write");
                goto end;
        }

        /* skip the last one if exists */
        if (entries > 3)
                entries = 3;

        for (i = 0; i < entries; i++)
        {
                int offset = (8 + 256 + 32) * i;
                if (write(fd, &buf[offset], (8 + 256 + 32)) != (8 + 256 + 32))
                {
                        LOG("error in write");
                        goto end;
                }
        }

        ret = 1;
end:
        close(fd);

        return ret;
}

static int data_initialize(struct ci_session *session)
{
        struct cc_ctrl_data *data;
        uint8_t buf[32], host_id[8];

        if (session->private_data)
        {

                LOG("strange private_data not null!");
                session->private_data = 0;
                //                return false;
        }

        data = calloc(1, sizeof(struct cc_ctrl_data));
        if (!data)
        {
                LOG("out of memory");
                return 0;
        }

        /* parent */
        data->session = session;

        /* clear storage of credentials */
        element_init(data);

        /* set status field - OK */
        memset(buf, 0, 1);
        if (!element_set(data, 30, buf, 1))
        {
                LOG("can not set status in elements");
        }

        /* set uri versions */
        memset(buf, 0, 32);
        buf[31] = session->ca->uri_mask; //uri version bitmask, e.g. 1-3
        LOG("uri version bitmask set to '%d'", buf[31]);
        if (!element_set(data, 29, buf, 32))
        {
                LOG("can not set uri_versions in elements");
        }
        /* load first AKH */
        data->akh_index = 0;
        if (!get_authdata(host_id, data->dhsk, buf, session->ca->id, data->akh_index, session->ca->ci_name))
        {
                //        if (!get_authdata(host_id, data->dhsk, buf, 1, data->akh_index)) {
                /* no AKH available */
                memset(buf, 0, sizeof(buf));
                data->akh_index = 5; /* last one */
        }

        if (!element_set(data, 22, buf, 32))
        {
                LOG("can not set AKH in elements");
        }

        if (!element_set(data, 5, host_id, 8))
        {
                LOG("can not set host_id elements");
        }

        session->private_data = data;

        return 1;
}
/* content_control commands */

static int sac_check_auth(const uint8_t *data, unsigned int len, uint8_t *sak)
{
        struct aes_xcbc_mac_ctx ctx;
        uint8_t calced_signature[16];

        if (len < 16)
        {
                LOG("auth too short");
                return 0;
        }

        aes_xcbc_mac_init(&ctx, sak);
        aes_xcbc_mac_process(&ctx, (uint8_t *)"\x04", 1); /* header len */
        aes_xcbc_mac_process(&ctx, data, len - 16);
        aes_xcbc_mac_done(&ctx, calced_signature);

        if (memcmp(&data[len - 16], calced_signature, 16))
        {
                LOG("signature wrong");
                return 0;
        }

        LOG("auth ok!");
        return 1;
}

static int sac_gen_auth(uint8_t *out, uint8_t *in, unsigned int len, uint8_t *sak)
{
        struct aes_xcbc_mac_ctx ctx;

        aes_xcbc_mac_init(&ctx, sak);
        aes_xcbc_mac_process(&ctx, (uint8_t *)"\x04", 1); /* header len */
        aes_xcbc_mac_process(&ctx, in, len);
        aes_xcbc_mac_done(&ctx, out);

        return 16;
}

static void generate_key_seed(struct cc_ctrl_data *cc_data)
{
        /* this is triggered by new ns_module */

        /* generate new key_seed -> SEK/SAK key derivation */

        SHA256_CTX sha;

        SHA256_Init(&sha);
        SHA256_Update(&sha, &cc_data->dhsk[240], 16);
        SHA256_Update(&sha, element_get_ptr(cc_data, 22), element_get_buf(cc_data, NULL, 22));
        SHA256_Update(&sha, element_get_ptr(cc_data, 20), element_get_buf(cc_data, NULL, 20));
        SHA256_Update(&sha, element_get_ptr(cc_data, 21), element_get_buf(cc_data, NULL, 21));
        SHA256_Final(cc_data->ks_host, &sha);
}

static void generate_ns_host(struct cc_ctrl_data *cc_data)

{
        uint8_t buf[8];
        get_random(buf, sizeof(buf));
        element_set(cc_data, 20, buf, sizeof(buf));
}

static int generate_SAK_SEK(uint8_t *sak, uint8_t *sek, const uint8_t *ks_host)
{
        AES_KEY key;
        const uint8_t key_data[16] = {0xea, 0x74, 0xf4, 0x71, 0x99, 0xd7, 0x6f, 0x35, 0x89, 0xf0, 0xd1, 0xdf, 0x0f, 0xee, 0xe3, 0x00};
        uint8_t dec[32];
        int i;

        /* key derivation of sak & sek */

        AES_set_encrypt_key(key_data, 128, &key);

        for (i = 0; i < 2; i++)
                AES_ecb_encrypt(&ks_host[16 * i], &dec[16 * i], &key, 1);

        for (i = 0; i < 16; i++)
                sek[i] = ks_host[i] ^ dec[i];

        for (i = 0; i < 16; i++)
                sak[i] = ks_host[16 + i] ^ dec[16 + i];

        return 0;
}

static int sac_crypt(uint8_t *dst, const uint8_t *src, unsigned int len, const uint8_t *key_data, int encrypt)
{
        uint8_t iv[16] = {0xf7, 0x70, 0xb0, 0x36, 0x03, 0x61, 0xf7, 0x96, 0x65, 0x74, 0x8a, 0x26, 0xea, 0x4e, 0x85, 0x41};
        AES_KEY key;

        /* AES_ENCRYPT is '1' */

        if (encrypt)
                AES_set_encrypt_key(key_data, 128, &key);
        else
                AES_set_decrypt_key(key_data, 128, &key);

        AES_cbc_encrypt(src, dst, len, &key, iv, encrypt);

        return 0;
}

static int verify_cb(int ok, X509_STORE_CTX *ctx)
{
        if (X509_STORE_CTX_get_error(ctx) == X509_V_ERR_CERT_NOT_YET_VALID)
        {
                time_t now = time(NULL);
                struct tm *t = localtime(&now);
                if (t->tm_year < 2015)
                {
                        LOG("seems our rtc is wrong - ignore!");
                        return 1;
                }
        }

        if (X509_STORE_CTX_get_error(ctx) == X509_V_ERR_CERT_HAS_EXPIRED)
                return 1;
        return 0;
}

static RSA *rsa_privatekey_open(const char *filename)
{
        FILE *fp;
        RSA *r = NULL;

        fp = fopen(filename, "r");
        if (!fp)
        {
                LOG("can not open %s", filename);
                return NULL;
        }

        PEM_read_RSAPrivateKey(fp, &r, NULL, NULL);
        if (!r)
        {
                LOG("read error");
        }

        fclose(fp);

        return r;
}

static X509 *certificate_open(const char *filename)
{
        FILE *fp;
        X509 *cert;

        fp = fopen(filename, "r");
        if (!fp)
        {
                LOG("can not open %s", filename);
                return NULL;
        }

        cert = PEM_read_X509(fp, NULL, NULL, NULL);
        if (!cert)
        {
                LOG("can not read cert");
        }

        fclose(fp);

        return cert;
}

static int certificate_validate(struct cert_ctx *ctx, X509 *cert)
{
        X509_STORE_CTX *store_ctx;
        int ret;

        store_ctx = X509_STORE_CTX_new();

        X509_STORE_CTX_init(store_ctx, ctx->store, cert, NULL);
        X509_STORE_CTX_set_verify_cb(store_ctx, verify_cb);
        X509_STORE_CTX_set_flags(store_ctx, X509_V_FLAG_IGNORE_CRITICAL);

        ret = X509_verify_cert(store_ctx);

        if (ret != 1)
        {
                LOG("%s", X509_verify_cert_error_string(X509_STORE_CTX_get_error(store_ctx)));
        }

        X509_STORE_CTX_free(store_ctx);

        if (ret == 1)
                return 1;
        else
                return 0;
}

static X509 *certificate_load_and_check(struct cert_ctx *ctx, const char *filename)
{
        X509 *cert;

        if (!ctx->store)
        {
                /* we assume this is the first certificate added - so its root-ca */
                ctx->store = X509_STORE_new();
                if (!ctx->store)
                {
                        LOG("can not create cert_store");
                        exit(-1);
                }

                if (X509_STORE_load_locations(ctx->store, filename, NULL) != 1)
                {
                        LOG("load of first certificate (root_ca) failed");
                        exit(-1);
                }

                return NULL;
        }

        cert = certificate_open(filename);
        if (!cert)
        {
                LOG("can not open certificate %s", filename);
                return NULL;
        }

        if (!certificate_validate(ctx, cert))
        {
                LOG("can not vaildate certificate");
                X509_free(cert);
                return NULL;
        }

        /* push into store - create a chain */
        if (X509_STORE_load_locations(ctx->store, filename, NULL) != 1)
        {
                LOG("load of certificate failed");
                X509_free(cert);
                return NULL;
        }

        return cert;
}

static X509 *certificate_import_and_check(struct cert_ctx *ctx, const uint8_t *data, int len)
{
        X509 *cert;

        cert = d2i_X509(NULL, &data, len);
        if (!cert)
        {
                LOG("can not read certificate");
                return NULL;
        }

        if (!certificate_validate(ctx, cert))
        {
                LOG("can not vaildate certificate");
                X509_free(cert);
                return NULL;
        }

        X509_STORE_add_cert(ctx->store, cert);

        return cert;
}

static X509 *import_ci_certificates(struct cc_ctrl_data *cc_data, unsigned int id)
{
        struct cert_ctx *ctx = cc_data->cert_ctx;
        X509 *cert;
        uint8_t buf[2048];
        unsigned int len;

        len = element_get_buf(cc_data, buf, id);

        cert = certificate_import_and_check(ctx, buf, len);
        if (!cert)
        {
                LOG("cannot read/verify DER cert");
                return NULL;
        }

        return cert;
}

static int check_ci_certificates(struct cc_ctrl_data *cc_data)
{
        struct cert_ctx *ctx = cc_data->cert_ctx;

        /* check if both certificates are available before we push and verify them */

        /* check for CICAM_BrandCert */
        if (!element_valid(cc_data, 8))
        {
                LOG("CICAM brand cert invalid");
                return -1;
        }

        /* check for CICAM_DevCert */
        if (!element_valid(cc_data, 16))
        {
                LOG("CICAM device cert invalid");
                return -1;
        }

        if (extract_ci_cert)
        {
                /* write ci device cert to disk */
                char ci_cert_file[256];
                memset(ci_cert_file, 0, sizeof(ci_cert_file));
                snprintf(ci_cert_file, sizeof(ci_cert_file) - 1, "/%s/ci_cert_%s_%d.der", getenv("HOME"), ci_name_underscore, ci_number);
                LOG("CI%d EXTRACTING %s", ci_number, ci_cert_file);
                int fd = open(ci_cert_file, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
                if (fd < 0)
                        LOG_AND_RETURN(-1, "opening %s for writing failed", ci_cert_file);
                int ret = write(fd, element_get_ptr(cc_data, 16), element_get_buf(cc_data, NULL, 16));
                if (ret)
                        LOG("write cert failed");
                close(fd);
                /* display strings in der cert file */
                cert_strings(ci_cert_file);
        }

        /* import CICAM_BrandCert */
        if ((ctx->ci_cust_cert = import_ci_certificates(cc_data, 8)) == NULL)
        {
                LOG("can not import brand cert");
                return -1;
        }

        /* import CICAM_DevCert */
        if ((ctx->ci_device_cert = import_ci_certificates(cc_data, 16)) == NULL)
        {
                LOG("can not import device cert");
                return -1;
        }

        /* everything seems to be fine here - so extract the CICAM_id from cert */
        if (!element_set_hostid_from_certificate(cc_data, 6, ctx->ci_device_cert))
        {
                LOG("can not set cicam_id in elements");
                return -1;
        }

        return 0;
}

static int generate_akh(struct cc_ctrl_data *cc_data)
{
        uint8_t akh[32];
        SHA256_CTX sha;

        SHA256_Init(&sha);
        SHA256_Update(&sha, element_get_ptr(cc_data, 6), element_get_buf(cc_data, NULL, 6));
        SHA256_Update(&sha, element_get_ptr(cc_data, 5), element_get_buf(cc_data, NULL, 5));
        SHA256_Update(&sha, cc_data->dhsk, 256);
        SHA256_Final(akh, &sha);

        element_set(cc_data, 22, akh, sizeof(akh));

        return 0;
}

static int check_dh_challenge(struct cc_ctrl_data *cc_data)
{
        /* check if every element for calculation of DHSK & AKH is available */
        LOG("checking ...");

        /* check for auth_nonce */
        if (!element_valid(cc_data, 19))
        {
                LOG("auth nonce invalid");
                return 0;
        }

        /* check for CICAM_id */
        if (!element_valid(cc_data, 6))
        {
                LOG("cicam id invalid");
                return 0;
        }

        /* check for DHPM */
        if (!element_valid(cc_data, 14))
        {
                LOG("dphm invalid");
                return 0;
        }

        /* check for Signature_B */
        if (!element_valid(cc_data, 18))
        {
                LOG("signature B invalid");
                return 0;
        }

        /* calculate DHSK - DHSK = DHPM ^ dh_exp % dh_p */
        dh_mod_exp(cc_data->dhsk, 256, element_get_ptr(cc_data, 14), 256, dh_p, sizeof(dh_p), cc_data->dh_exp, 256);

        /* gen AKH */
        generate_akh(cc_data);

        /* disable 5 tries of startup -> use new calculated one */
        cc_data->akh_index = 5;

        LOG("writing authdata ...");
        /* write to disk */
        write_authdata(cc_data->session->ca->id, element_get_ptr(cc_data, 5), cc_data->dhsk, element_get_ptr(cc_data, 22), cc_data->session->ca->ci_name);

        return 1;
}

static int restart_dh_challenge(struct cc_ctrl_data *cc_data)
{
        uint8_t dhph[256], sign_A[256];
        struct cert_ctx *ctx;
        LOG(".... rechecking ...");

        if (!cc_data->cert_ctx)
        {
                ctx = calloc(1, sizeof(struct cert_ctx));
                cc_data->cert_ctx = ctx;
        }
        else
        {
                ctx = cc_data->cert_ctx;
        }

        /* load certificates and device key */
        certificate_load_and_check(ctx, "/etc/ssl/certs/root.pem");
        ctx->cust_cert = certificate_load_and_check(ctx, "/etc/ssl/certs/customer.pem");
        ctx->device_cert = certificate_load_and_check(ctx, "/etc/ssl/certs/device.pem");

        if (!ctx->cust_cert || !ctx->device_cert)
        {
                LOG("can not check loader certificates");
                return -1;
        }

        /* add data to element store */
        if (!element_set_certificate(cc_data, 7, ctx->cust_cert))
                LOG("can not store cert in elements");

        if (!element_set_certificate(cc_data, 15, ctx->device_cert))
                LOG("can not store cert in elements");

        if (!element_set_hostid_from_certificate(cc_data, 5, ctx->device_cert))
                LOG("can not set hostid in elements");

        cc_data->rsa_device_key = rsa_privatekey_open("/etc/ssl/certs/device.pem");
        if (!cc_data->rsa_device_key)
        {
                LOG("can not read private key");
                return -1;
        }

        /* invalidate elements */
        element_invalidate(cc_data, 6);
        element_invalidate(cc_data, 14);
        element_invalidate(cc_data, 18);
        element_invalidate(cc_data, 22); /* this will refuse a unknown cam */

        /* new dh_exponent */
        dh_gen_exp(cc_data->dh_exp, 256, dh_g, sizeof(dh_g), dh_p, sizeof(dh_p));

        /* new DHPH  - DHPH = dh_g ^ dh_exp % dh_p */
        dh_mod_exp(dhph, sizeof(dhph), dh_g, sizeof(dh_g), dh_p, sizeof(dh_p), cc_data->dh_exp, 256);

        /* store DHPH */
        element_set(cc_data, 13, dhph, sizeof(dhph));

        /* create Signature_A */
        dh_dhph_signature(sign_A, element_get_ptr(cc_data, 19), dhph, cc_data->rsa_device_key);

        /* store Signature_A */
        element_set(cc_data, 17, sign_A, sizeof(sign_A));

        return 0;
}

static int generate_uri_confirm(struct cc_ctrl_data *cc_data, const uint8_t *sak)
{
        SHA256_CTX sha;
        uint8_t uck[32];
        uint8_t uri_confirm[32];

        /* calculate UCK (uri confirmation key) */
        SHA256_Init(&sha);
        SHA256_Update(&sha, sak, 16);
        SHA256_Final(uck, &sha);

        /* calculate uri_confirm */
        SHA256_Init(&sha);
        SHA256_Update(&sha, element_get_ptr(cc_data, 25), element_get_buf(cc_data, NULL, 25));
        SHA256_Update(&sha, uck, 32);
        SHA256_Final(uri_confirm, &sha);

        element_set(cc_data, 27, uri_confirm, 32);

        return 0;
}

static void check_new_key(ca_device_t *d, struct cc_ctrl_data *cc_data)
{
        const uint8_t s_key[16] = {0x3e, 0x20, 0x15, 0x84, 0x2c, 0x37, 0xce, 0xe3, 0xd6, 0x14, 0x57, 0x3e, 0x3a, 0xab, 0x91, 0xb6};
        AES_KEY aes_ctx;
        uint8_t dec[32];
        uint8_t *kp;
        uint8_t slot;
        unsigned int i;

        /* check for keyprecursor */
        if (!element_valid(cc_data, 12))
        {
                LOG("key precursor invalid");
                return;
        }

        /* check for slot */
        if (!element_valid(cc_data, 28))
        {
                LOG("slot(key register) invalid");
                return;
        }
        kp = element_get_ptr(cc_data, 12);
        element_get_buf(cc_data, &slot, 28);

        AES_set_encrypt_key(s_key, 128, &aes_ctx);
        for (i = 0; i < 32; i += 16)
                AES_ecb_encrypt(&kp[i], &dec[i], &aes_ctx, 1);

        for (i = 0; i < 32; i++)
                dec[i] ^= kp[i];

        LOGM("=== descrambler_set_key === adapter = CA%i key regiser (0-even, 1-odd) = %i", cc_data->session->ca->id, slot);
        char buf[400];
        int pos;
        pos = sprintf(buf, "KEY: ");
        for (i = 0; i < 16; i++)
                pos += sprintf(buf + pos, "%02X ", dec[i]);
        pos += sprintf(buf + pos, "\n\t\t\t\t\t\tIV: ");
        for (i = 16; i < 32; i++)
                pos += sprintf(buf + pos, "%02X ", dec[i]);
        LOG("received from CI+ CAM %d: %s", d->id, buf);

        memcpy(d->key[slot], dec, 16);
        memcpy(d->iv[slot], dec + 16, 16);
        d->parity = slot;

        for (i = 0; i < MAX_CA_PMT; i++)
                if (d->pmt_id[i] != -1)
                {
                        send_cw(d->pmt_id[i], CA_ALGO_AES128_CBC, slot, d->key[slot], d->iv[slot], 3720);
                }
        d->is_ciplus = 1;

        /* reset */
        element_invalidate(cc_data, 12);
        element_invalidate(cc_data, 28);
}

static int data_get_handle_new(ca_device_t *d, struct cc_ctrl_data *cc_data, unsigned int id)
{
        /* handle trigger events */

        /* depends on new received items */
        //LOG("!!!!!!!!!!!!!!!!!!!! data_get_handle_new ID = %i!!!!!!!!!!!!!!!!!!!!!!!!!", id);
        switch (id)
        {
        case 8: /* CICAM_BrandCert */
                /* this results in CICAM_ID when cert-chain is verified and ok */
                if (check_ci_certificates(cc_data))
                        break;
                /* generate DHSK & AKH */
                check_dh_challenge(cc_data);
                break;

        case 19: /* auth_nonce - triggers new dh keychallenge - invalidates DHSK & AKH */
                /* generate DHPH & Signature_A */
                restart_dh_challenge(cc_data);
                break;

        case 21: /* Ns_module - triggers SAC key calculation */
                generate_ns_host(cc_data);
                generate_key_seed(cc_data);
                generate_SAK_SEK(cc_data->sak, cc_data->sek, cc_data->ks_host);
                break;

                /* SAC data messages */

        case 28: /* key register */
                check_new_key(d, cc_data);
                break;
        case 25: //uri_message
                 //        case 26:        /* program_number */
                generate_uri_confirm(cc_data, cc_data->sak);
                break;

        case 6:  /* CICAM_id */
        case 12: /* keyprecursor */
        case 14: /* DHPM */
        case 16: /* CICAM_DevCert */
        case 18: /* Signature_B */
        case 26: /* program_number */
                LOG("not need to be handled id %d", id);
                break;

        default:
                LOG("unhandled id %d", id);
                break;
        }

        return 0;
}

static int data_req_handle_new(struct cc_ctrl_data *cc_data, unsigned int id)
{
        switch (id)
        {
        case 22: /* AKH */
        {
                uint8_t akh[32], host_id[8];
                memset(akh, 0, sizeof(akh));
                if (cc_data->akh_index != 5)
                {
                        if (!get_authdata(host_id, cc_data->dhsk, akh, cc_data->session->ca->id, cc_data->akh_index++, cc_data->session->ca->ci_name))
                                cc_data->akh_index = 5;
                        if (!element_set(cc_data, 22, akh, 32))
                                LOG("cannot set AKH in elements");
                        if (!element_set(cc_data, 5, host_id, 8))
                                LOG("cannot set host_id in elements");
                }
        }
        default:
                break;
        }

        return 0;
}

static int data_get_loop(ca_device_t *d, struct cc_ctrl_data *cc_data, const unsigned char *data, unsigned int datalen, unsigned int items)
{
        unsigned int i;
        int dt_id, dt_len;
        unsigned int pos = 0;

        for (i = 0; i < items; i++)
        {
                if (pos + 3 > datalen)
                        return 0;
                dt_id = data[pos++];
                dt_len = data[pos++] << 8;
                dt_len |= data[pos++];
                if (pos + dt_len > datalen)
                        return 0;
                LOG("set element(dt_id) %d dt_len = %i", dt_id, dt_len);
                //                hexdump("data_get_loop: ", (void *)&data[pos], dt_len);
                element_set(cc_data, dt_id, &data[pos], dt_len);
                data_get_handle_new(d, cc_data, dt_id);

                pos += dt_len;
        }

        return pos;
}

static int data_req_loop(struct cc_ctrl_data *cc_data, unsigned char *dest, const unsigned char *data, unsigned int datalen, unsigned int items)
{
        int dt_id;
        unsigned int i;
        int pos = 0;
        int len;

        if (items > datalen)
                return -1;

        for (i = 0; i < items; i++)
        {
                dt_id = *data++;
                LOG("req element %d", dt_id);
                data_req_handle_new(cc_data, dt_id); /* check if there is any action needed before we answer */
                len = element_get_req(cc_data, dest, dt_id);
                if (len == 0)
                {
                        LOG("cannot get element %d", dt_id);
                        return -1;
                }
                pos += len;
                dest += len;
        }

        return pos;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int buildLengthField(uint8_t *pkt, int len)
{
        if (len < 127)
        {
                *pkt++ = len;
                return 1;
        }
        else if (len < 256)
        {
                *pkt++ = 0x81;
                *pkt++ = len;
                return 2;
        }
        else if (len < 65536)
        {
                *pkt++ = 0x82;
                *pkt++ = len >> 8;
                *pkt++ = len;
                return 3;
        }
        else
        {
                LOG("too big for lengthField");
                exit(0);
        }
}

static void ci_session_sendSPDU_A(struct ca_device *ca, uint8_t tag, uint8_t *data, size_t len, uint16_t session_nb)
{
        uint8_t pkt[4096];
        uint8_t *ptr = pkt;

        *ptr++ = tag;
        ptr += buildLengthField(ptr, len + 2);
        *ptr++ = session_nb >> 8;
        *ptr++ = session_nb;

        if (data)
                memcpy(ptr, data, len);
        ptr += len;
        //        LOG("==============ci_session_sendSPDU_A================ sess = %i sess_nb = %i len = %i ",ca->session,ca->session->index,len);
        //        hexdump("ci_session_sendSPDU_A  DATA ", data, (len < 50 ? len : 16));
        en50221_sl_send_data(ca->sl, session_nb, data, len);
}

void ci_session_sendAPDU(struct ci_session *session, const uint8_t *tag, const uint8_t *data, size_t len)
{
        uint8_t pkt[len + 3 + 4];
        int l;

        memcpy(pkt, tag, 3);
        l = buildLengthField(pkt + 3, len);
        if (data)
                memcpy(pkt + 3 + l, data, len);
        ci_session_sendSPDU_A(session->ca, 0x90, pkt, len + 3 + l, session->index);
}

void ci_ccmgr_cc_open_cnf(struct ci_session *session)
{
        const uint8_t tag[3] = {0x9f, 0x90, 0x02};
        const uint8_t bitmap = 0x01;

        data_initialize(session);
        LOG("SEND ------------ CC_OPEN_CNF----------- ");
        ci_session_sendAPDU(session, tag, &bitmap, 1);
}

static int ci_ccmgr_cc_sac_send(struct ci_session *session, const uint8_t *tag, uint8_t *data, unsigned int pos)
{
        struct cc_ctrl_data *cc_data = session->private_data;
        if (pos < 8)
                return 0;
        LOG("______________________ci_ccmgr_cc_sac_send______________________");
        //	_hexdump("TAG:   ", &tag, 3);
        //	_hexdump("UNENCRYPTED:  ", data, pos);

        pos += add_padding(&data[pos], pos - 8, 16);
        BYTE16(&data[6], pos - 8); /* len in header */

        pos += sac_gen_auth(&data[pos], data, pos, cc_data->sak);
        sac_crypt(&data[8], &data[8], pos - 8, cc_data->sek, AES_ENCRYPT);

        //        _hexdump("ENCRYPTED    ",data, pos);
        ci_session_sendAPDU(session, tag, data, pos);

        return 1;
}

static int ci_ccmgr_cc_sac_data_req(ca_device_t *d, struct ci_session *session, const uint8_t *data, unsigned int len)
{
        struct cc_ctrl_data *cc_data = session->private_data;
        const uint8_t data_cnf_tag[3] = {0x9f, 0x90, 0x08};
        uint8_t dest[2048];
        uint8_t tmp[len];
        int id_bitmask, dt_nr;
        unsigned int serial;
        int answ_len;
        int pos = 0;
        unsigned int rp = 0;

        if (len < 10)
                return 0;
        //_hexdump("ci_ccmgr_cc_sac_data_req:", data, len);

        memcpy(tmp, data, 8);
        sac_crypt(&tmp[8], &data[8], len - 8, cc_data->sek, AES_DECRYPT);
        data = tmp;

        if (!sac_check_auth(data, len, cc_data->sak))
        {
                LOG("check_auth of message failed");
                return 0;
        }

        serial = UINT32(&data[rp], 4);
        LOG("serial sac data req: %d", serial);

        /* skip serial & header */
        rp += 8;

        id_bitmask = data[rp++];

        /* handle data loop */
        dt_nr = data[rp++];
        rp += data_get_loop(d, cc_data, &data[rp], len - rp, dt_nr);

        if (len < rp + 1)
                return 0;

        dt_nr = data[rp++];

        /* create answer */
        pos += BYTE32(&dest[pos], serial);
        pos += BYTE32(&dest[pos], 0x01000000);

        dest[pos++] = id_bitmask;
        dest[pos++] = dt_nr; /* dt_nbr */

        answ_len = data_req_loop(cc_data, &dest[pos], &data[rp], len - rp, dt_nr);
        if (answ_len <= 0)
        {
                LOG("cannot req data");
                return 0;
        }
        pos += answ_len;

        LOG("SEND ------------ CC_SAC_DATA_CNF----------- ");
        //        _hexdump("sac_data_send", &dest[8], pos-8);  //skip serial and header
        return ci_ccmgr_cc_sac_send(session, data_cnf_tag, dest, pos);
}

static void ci_ccmgr_cc_sac_sync_req(struct ci_session *session, const uint8_t *data, unsigned int len)
{
        const uint8_t sync_cnf_tag[3] = {0x9f, 0x90, 0x10};
        uint8_t dest[64];
        unsigned int serial;
        int pos = 0;

        //      hexdump("cc_sac_sync_req: ", (void *)data, len);

        serial = UINT32(data, 4);

        pos += BYTE32(&dest[pos], serial);
        pos += BYTE32(&dest[pos], 0x01000000);

        /* status OK */
        dest[pos++] = 0;

        LOG("SEND ------------ CC_SAC_SYNC_CNF----------- ");
        ci_ccmgr_cc_sac_send(session, sync_cnf_tag, dest, pos);
}

static void ci_ccmgr_cc_sync_req(struct ci_session *session, const uint8_t *data, unsigned int len)
{
        const uint8_t tag[3] = {0x9f, 0x90, 0x06};
        const uint8_t status = 0x00; /* OK */
        LOG("SEND ------------ CC_SYNC_CNF----------- ");
        ci_session_sendAPDU(session, tag, &status, 1);
}

static int ci_ccmgr_cc_data_req(ca_device_t *d, struct ci_session *session, const uint8_t *data, unsigned int len)
{
        struct cc_ctrl_data *cc_data = session->private_data;
        uint8_t cc_data_cnf_tag[3] = {0x9f, 0x90, 0x04};
        uint8_t dest[2048 * 2];
        int dt_nr;
        int id_bitmask;
        int answ_len;
        unsigned int rp = 0;

        if (len < 2)
                return 0;

        id_bitmask = data[rp++];

        /* handle data loop */
        dt_nr = data[rp++];

        //printf("CC_DATA:   ");
        //hexdump(cc_data, sizeof(cc_data));
        rp += data_get_loop(d, cc_data, &data[rp], len - rp, dt_nr);

        if (len < rp + 1)
                return 0;

        /* handle req_data loop */
        dt_nr = data[rp++];

        dest[0] = id_bitmask;
        dest[1] = dt_nr;

        answ_len = data_req_loop(cc_data, &dest[2], &data[rp], len - rp, dt_nr);
        if (answ_len <= 0)
        {
                LOG("cannot req data");
                return 0;
        }

        answ_len += 2;

        LOG("SEND ------------ CC_DATA_CNF----------- ");
        ci_session_sendAPDU(session, cc_data_cnf_tag, dest, answ_len);

        return 1;
}

void *
stackthread_func(void *arg)
{
        ca_device_t *d = arg;
        int lasterror = 0;
        adapter *ad;
        sprintf(thread_name, "CA%d", d->id);

        LOG("%s: start", __func__);

        while (d->enabled)
        {
                usleep(100 * 1000);
                int error;
                if ((error = en50221_tl_poll(d->tl)) != 0)
                {
                        if (error != lasterror && en50221_tl_get_error(d->tl) != -7)
                        {
                                LOG("Error reported by stack slot:%i error:%i",
                                    en50221_tl_get_error_slot(d->tl),
                                    en50221_tl_get_error(d->tl));
                                ad = get_adapter(d->id);
                                if (ad)
                                        ad->adapter_timeout = opts.adapter_timeout;
                                d->ignore_close = 0; // force device close..
                                                     //ca_init(d);
                                                     //				break;
                        }
                        lasterror = error;
                }
        }

        return 0;
}

static int ciplus_app_cc_message(void *arg, uint8_t slot_id, uint16_t session_number, uint32_t resource_id, uint8_t *data, uint32_t data_length)
{
        ca_device_t *d = arg;
        d->session->index = session_number;
        d->session->resid = resource_id;
        d->session->ca = d;
        d->slot_id = slot_id;

        uint32_t tag;
        uint32_t len;

        //printf(" RECV DATA WITH TAG:   ");
        //hexdump(" RECV DATA WITH TAG:   ", data,data_length<33?data_length:32);

        tag = data[0] << 16 | data[1] << 8 | data[2];
        if (data[3] == 0x82)
        {
                len = (data[4] << 8 | data[5]);
                data = &data[6];
        }
        else if (data[3] == 0x81)
        {
                len = (data[4] << 8);
                data = &data[5];
        }
        else
        {
                len = data[3];
                data = &data[4];
        }
        LOG("RECV ciplus cc msg CAM%i slot_id %u, session_num %u, resource_id %x tag %x len %i dt_id %i",
            d->id, slot_id, session_number, resource_id, tag, len, data[2]);
        //printf(" RECV DATA:   ");
        //hexdump(data,len<33?len:32);

        switch (tag)
        {

        case CIPLUS_TAG_CC_OPEN_REQ: //01
                ci_ccmgr_cc_open_cnf(d->session);
                break;
        case CIPLUS_TAG_CC_DATA_REQ: //03
                ci_ccmgr_cc_data_req(d, d->session, data, len);
                break;
        case CIPLUS_TAG_CC_SYNC_REQ: //05
                ci_ccmgr_cc_sync_req(d->session, data, len);
                break;
        case CIPLUS_TAG_CC_SAC_DATA_REQ: //07
                ci_ccmgr_cc_sac_data_req(d, d->session, data, len);
                break;
        case CIPLUS_TAG_CC_SAC_SYNC_REQ: //09
                ci_ccmgr_cc_sac_sync_req(d->session, data, len);
                break;
        default:
                LOG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! unknown cc tag %x len %u", tag, len);
        }
        return 0;
}

static int ciplus_app_lang_message(void *arg, uint8_t slot_id, uint16_t session_number, uint32_t resource_id, uint8_t *data, uint32_t data_length)
{
        ca_device_t *d = arg;
        uint32_t tag;

        LOG("host_lang&country_receive");
        //        if (data_length)
        //                hexdump(data, data_length);

        tag = data[0] << 16 | data[1] << 8 | data[2];

        uint8_t data_reply_lang[3]; // ISO 639 Part 2
        data_reply_lang[0] = 0x65;  /* e */
        data_reply_lang[1] = 0x6e;  /* n */
        data_reply_lang[2] = 0x67;  /* g */

        uint8_t data_reply_country[3]; // ISO 3166-1 alpha 3
        data_reply_country[0] = 0x55;  /* U */
        data_reply_country[1] = 0x53;  /* S */
        data_reply_country[2] = 0x41;  /* A */

        switch (tag)
        {
        case CIPLUS_TAG_COUNTRY_ENQ: /* country enquiry */
        {
                LOG("country answered with '%c%c%c'", data_reply_country[0], data_reply_country[1], data_reply_country[2]);
                uint8_t tag[3] = {0x9f, 0x81, 0x01}; /* host country reply */
                ci_session_sendAPDU(d->session, tag, data_reply_country, 3);
                break;
        }
        case CIPLUS_TAG_LANG_ENQ: /* language enquiry */
        {
                LOG("language answered with '%c%c%c'", data_reply_lang[0], data_reply_lang[1], data_reply_lang[2]);
                uint8_t tag[3] = {0x9f, 0x81, 0x11}; /* host language reply */
                ci_session_sendAPDU(d->session, tag, data_reply_lang, 3);
                break;
        }
        default:
                LOG("unknown host lac apdu tag %02x", tag);
        }
        return 0;
}

static int ciplus_app_sas_message(void *arg, uint8_t slot_id, uint16_t session_number, uint32_t resource_id, uint8_t *data, uint32_t data_length)
{
        ca_device_t *d = arg;
        uint32_t tag;
        LOG("CAM_SAS_connect_cnf_receive");

        tag = data[0] << 16 | data[1] << 8 | data[2];

        switch (tag)
        {
        case CIPLUS_TAG_SAS_CONNECT_CNF: /* */
        {
                LOG("CI+ CA%i 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x  0x%02x",
                    d->id, data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11], data[12]);
                if (data[12] == 0)
                {
                        uint8_t tag[3] = {0x9f, 0x9a, 0x07}; /* cam sas async msg */
                        ci_session_sendAPDU(d->session, tag, 0x00, 0);
                }
                break;
        }
        default:
                LOG("unknown SAS apdu tag %03x", tag);
        }

        return 0;
} /* not working, just for fun */

static int ciplus_app_oprf_message(void *arg, uint8_t slot_id, uint16_t session_number, uint32_t resource_id, uint8_t *data, uint32_t data_length)
{
        ca_device_t *d = arg;
        uint32_t tag;
        char buf[400];
        int pos, i;
        pos = sprintf(buf, " ");

        uint8_t data_oprf_search[9];
        data_oprf_search[0] = 0x03; /* unattended mode bit=0 + length in bytes of the service types */
        data_oprf_search[1] = 0x01; /* service MPEG-2 television (0x01) */
        data_oprf_search[2] = 0x16; /* service h264 SD (0x16) */
        data_oprf_search[3] = 0x19; /* service h264 HD (0x19) */
        data_oprf_search[4] = 0x02; /* length in bytes of the delivery_capability */
        data_oprf_search[5] = 0x43; /* DVB-S */
        data_oprf_search[6] = 0x79; /* DVB-S2 */
        data_oprf_search[7] = 0x01; /* length in bytes of the application_capability */
        data_oprf_search[8] = 0x00; /* System Software Update service */

        uint8_t data_oprf_tune_status[64];
        data_oprf_tune_status[0] = 0x01; //unprocessed descriptor_number
        data_oprf_tune_status[1] = 0x50; //signal strength
        data_oprf_tune_status[2] = 0x50; //signal quality
        data_oprf_tune_status[3] = 0x00; //status 0 - OK
        data_oprf_tune_status[4] = 0x0d; //lenght next part (13 bytes)

        tag = data[0] << 16 | data[1] << 8 | data[2];

        switch (tag)
        {
        case CIPLUS_TAG_OPERATOR_STATUS: /* operator_status 01 */
        {
                LOG("CAM_OPRF_operator_status_receive");
                pos = sprintf(buf, " ");
                for (i = 4; i < data[3] + 4; i++)
                        pos += sprintf(buf + pos, "%02X ", data[i]);
                uint8_t tag_part = 0x04; /* operator_info_req */
                if (data[5] & 0x20)      //initialised_flag - profile initialised
                        tag_part = 0x08; /* operator_exit */
                if (data[5] & 0x40)
                {
                        LOG("CI+ CA%d: %s operator profile %sinitialised", d->id, buf, data[5] & 0x60 ? "" : "NOT ");
                }
                else
                        LOG("CI+ CA%d: %s operator profile disabled", d->id, buf);
                uint8_t tag[3] = {0x9f, 0x9c, tag_part};
                ci_session_sendAPDU(d->session, tag, 0x00, 0);
                break;
        }
        case CIPLUS_TAG_OPERATOR_INFO: /* operator_info */
        {
                LOG("CAM_OPRF_operator_info_receive");
                for (i = 4; i < data[3] + 4; i++)
                        pos += sprintf(buf + pos, "%02X ", data[i]);
                LOG("CI+ CA%d: %s", d->id, buf);
                uint8_t tag[3] = {0x9f, 0x9c, 0x06}; /* operator_search_start */
                ci_session_sendAPDU(d->session, tag, data_oprf_search, 9);
                break;
        }
        case 0x9f9c03: /* operator_nit */
        {
                LOG("CAM_OPRF_operator_nit_receive");
                for (i = 4; i < data[3] + 4; i++)
                        pos += sprintf(buf + pos, "%02X ", data[i]);
                LOG("CI+ CA%d: %s", d->id, buf);
                uint8_t tag[3] = {0x9f, 0x9c, 0x08}; /* operator_exit */
                ci_session_sendAPDU(d->session, tag, 0x00, 0);
                break;
        }
        case CIPLUS_TAG_OPERATOR_SEARCH_STATUS: /* operator_search_status */
        {
                LOG("CAM_OPRF_operator_search_status_receive");
                for (i = 4; i < data[3] + 4; i++)
                        pos += sprintf(buf + pos, "%02X ", data[i]);
                LOG("CI+ CA%d: %s", d->id, buf);

                uint8_t tag[3] = {0x9f, 0x9c, 0x02}; /* operator_nit */
                if (data[5] & 0x02)                  // refresh_request_flag == 2 (urgent request)
                {
                        uint8_t tag[3] = {0x9f, 0x9c, 0x06}; /* operator_search_start */
                        ci_session_sendAPDU(d->session, tag, data_oprf_search, 9);
                }
                else
                        ci_session_sendAPDU(d->session, tag, 0x00, 0);
                break;
        }
        case CIPLUS_TAG_OPERATOR_TUNE: /* operator_tune */
        {
                char *pol = "H";
                if (data[9] & 0x20)
                        pol = "V";
                else if (data[9] & 0x40)
                        pol = "L";
                else if (data[9] & 0x60)
                        pol = "R";
                LOG("CAM_OPRF_operator_tune_receive");
                for (i = 4; i < data[3] + 4; i++)
                {
                        pos += sprintf(buf + pos, "%02X ", data[i]);
                        data_oprf_tune_status[i + 1] = data[i + 2];
                }
                LOG("CI+ CA%d: %s", d->id, buf);
                LOG("Please TUNE to transponder %x%x%x %c", data[6], data[7], data[8], *pol);
                //data_oprf_tune_status[13]=0xC6; //psk8 dvb-s2
                usleep(3 * 1000 * 1000);             //wait 3 secs
                uint8_t tag[3] = {0x9f, 0x9c, 0x0a}; /* operator_tune_status */
                ci_session_sendAPDU(d->session, tag, data_oprf_tune_status, 18);
                break;
        }
        default:
                LOG("unknown OPRF apdu tag %03x", tag);
        }
        return 0;
}

static int ciplus_app_upgr_message(void *arg, uint8_t slot_id, uint16_t session_number, uint32_t resource_id, uint8_t *data, uint32_t data_length)
{
        ca_device_t *d = arg;
        int fd = d->fd;
        ca_slot_info_t info;

        uint32_t tag;
        const uint8_t answer = 0x00; // 0x00 - mean no upgrade, 0x01 - upgrade, 0x02 - ask user by mmi

        LOG("CAM_fw_upgrade_receive");

        tag = data[0] << 16 | data[1] << 8 | data[2];

        switch (tag)
        {
        case CIPLUS_TAG_FIRMWARE_UPGR: /* */
        {
                LOG("CI+ CA%i Firmware Upgrade Command detected... ", d->id);
                uint8_t tag[3] = {0x9f, 0x9d, 0x02}; /* cam firmware update reply */
                ci_session_sendAPDU(d->session, tag, &answer, 1);
                break;
        }
        case CIPLUS_TAG_FIRMWARE_UPGR_PRGRS: /* */
        {
                LOG("CI+ CA%i Firmware Upgrade Progress %i percents", d->id, data[4]);
                break;
        }
        case CIPLUS_TAG_FIRMWARE_UPGR_COMPLT: /* */
        {
                LOG("CI+ CA%i Firmware Upgrade Complete (reset status %i)", d->id, data[4]);
                if (data[4] < 2)
                { //reset requred
                        if (ioctl(fd, CA_RESET, &info))
                                LOG_AND_RETURN(0, "%s: Could not reset ca %d", __FUNCTION__, d->id);
                        return 1;
                }
                break;
        }
        default:
                LOG("unknown fw upgrade apdu tag %03x", tag);
        }
        return 0;
} /* works now, be careful!!! just in case upgrade disabled */

static int ciplus_app_ai_message(void *arg, uint8_t slot_id, uint16_t session_number, uint32_t resource_id, uint8_t *data, uint32_t data_length)
{
        ca_device_t *d = arg;
        ca_slot_info_t info;
        int fd = d->fd;

        if (data_length < 3)
        {
                LOG("Received short data");
                return -1;
        }
        uint32_t tag = (data[0] << 16) | (data[1] << 8) | data[2];

        LOG("RECV ciplus AI msg slot_id %u, session_num %u, resource_id %x tag %x",
            slot_id, session_number, resource_id, tag);

        switch (tag)
        {
        case CIPLUS_TAG_APP_INFO:
                //		hexdump(data,data_length);
                return en50221_app_ai_message(d->ai_resource, slot_id,
                                              session_number,
                                              resource_id,
                                              data,
                                              data_length);
                break;
        case CIPLUS_TAG_CICAM_RESET:
                //		hexdump(data,data_length);
                if (ioctl(fd, CA_RESET, &info))
                        LOG_AND_RETURN(0, "%s: Could not reset ca %d", __FUNCTION__, d->id);
                return 1;
                break;
        default:
                LOG("Received unexpected tag %x", tag);
        }
        return -1;
}

static int en50221_app_unknown_message(void *arg, uint8_t slot_id, uint16_t session_number, uint32_t resource_id, uint8_t *data, uint32_t data_length)
{
        ca_device_t *d = arg;
        d->session->index = session_number;
        d->session->resid = resource_id;
        d->slot_id = slot_id;
        uint32_t tag = data[0] << 16 | data[1] << 8 | data[2];
        LOG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  unknown message slot_id %u, session_num %u, resource_id %x tag %u",
            slot_id, session_number, resource_id, tag);
        return -1;
}

static int ca_session_callback(void *arg, int reason, uint8_t slot_id,
                               uint16_t session_number, uint32_t resource_id)
{
        ca_device_t *d = arg;
        d->session->ca = d;

        LOG("%s: reason %d slot_id %u session_number %u resource_id %x", __func__,
            reason, slot_id, session_number, resource_id);

        switch (reason)
        {
        case S_SCALLBACK_REASON_CAMCONNECTING: //0
                LOG("%02x:CAM connecting to resource %08x, session_number %i",
                    slot_id, resource_id, session_number);
                break;
        case S_SCALLBACK_REASON_CLOSE: //5
                LOG("%02x:Connection to resource %08x, session_number %i closed",
                    slot_id, resource_id, session_number);

                if (resource_id == EN50221_APP_CA_RESOURCEID)
                {
                        LOG("_________S_SCALLBACK_REASON_CLOSE___________EN50221_APP_CA_RESOURCEID__________________________");
                        d->ignore_close = 1;
                        d->init_ok = 0;
                }
                else if (resource_id == CIPLUS_APP_CC_RESOURCEID || resource_id == CIPLUS_APP_CC_RESOURCEID_TWO || resource_id == CIPLUS_APP_CC_RESOURCEID_THREE || resource_id == CIPLUS_APP_CC_RESOURCEID_MULTI)
                {
                        LOG("__________S_SCALLBACK_REASON_CLOSE____________CIPLUS_APP_CC_RESOURCEID________________________");
                }
                else if (resource_id == EN50221_APP_MMI_RESOURCEID)
                {
                        LOG("_________S_SCALLBACK_REASON_CLOSE____________EN50221_APP_MMI_RESOURCEID_________________________");
                }
                break;

        case S_SCALLBACK_REASON_TC_CONNECT: //6
                LOG("%02x:Host originated transport connection %i resource 0x%08x connected", slot_id, resource_id, session_number);
                break;
        case S_SCALLBACK_REASON_TC_CAMCONNECT: //7
                LOG("%02x:CAM originated transport connection %i connected", slot_id, session_number);
                break;
        case S_SCALLBACK_REASON_CAMCONNECTED: //1
                LOG("%02x:CAM successfully connected to resource %08x, session_number %i",
                    slot_id, resource_id, session_number);

                if (resource_id == EN50221_APP_RM_RESOURCEID)
                {
                        LOG("--------------------S_SCALLBACK_REASON_CAMCONNECTED---------EN50221_APP_RM_RESOURCEID-------------------------");
                        en50221_app_rm_enq(d->rm_resource, session_number);
                }
                else if (resource_id == EN50221_APP_AI_RESOURCEID ||
                         resource_id == TS101699_APP_AI_RESOURCEID)
                {
                        LOG("--------------------S_SCALLBACK_REASON_CAMCONNECTED---------EN50221_APP_AI_RESOURCEID-------------------------");
                        d->ai_session_number = session_number;
                        en50221_app_ai_enquiry(d->ai_resource, session_number);
                }
                else if (resource_id == CIPLUS_APP_AI_RESOURCEID)
                {
                        LOG("--------------------S_SCALLBACK_REASON_CAMCONNECTED---------CIPLUS_APP_AI_RESOURCEID-------------------------");
                        d->ca_ai_version = resource_id & 0x3f;
                        d->ai_session_number = session_number;
                        d->ca_high_bitrate_mode = 1; // 96 MBPS now (should get from command line)
                        en50221_app_ai_enquiry(d->ai_resource, session_number);
                        ciplus13_app_ai_data_rate_info(d, d->ca_high_bitrate_mode ? CIPLUS_DATA_RATE_96_MBPS : CIPLUS_DATA_RATE_72_MBPS);
                }
                else if (resource_id == EN50221_APP_CA_RESOURCEID ||
                         resource_id == CIPLUS_APP_CA_RESOURCEID)
                {
                        LOG("--------------------S_SCALLBACK_REASON_CAMCONNECTED---------EN50221_APP_CA_RESOURCEID-------------------------");
                        en50221_app_ca_info_enq(d->ca_resource, session_number);
                        d->ca_session_number = session_number;
                }
                else if (resource_id == EN50221_APP_MMI_RESOURCEID)
                {
                        LOG("--------------------S_SCALLBACK_REASON_CAMCONNECTED---------EN50221_APP_MMI_RESOURCEID-------------------------");
                }
                else if (resource_id == CIPLUS_APP_SAS_RESOURCEID)
                {
                        LOG("--------------------S_SCALLBACK_REASON_CAMCONNECTED---------CIPLUS_APP_SAS_RESOURCEID-------------------------");
                        uint8_t data[] = {0x9f, 0x9a, 0x00, 0x08, 0x69, 0x74, 0x64, 0x74, 0x74, 0x63, 0x61, 0x00}; //blank private_Host_application_ID
                        en50221_sl_send_data(d->sl, session_number, data, sizeof(data));
                }
                else if (resource_id == CIPLUS_APP_OPRF_RESOURCEID)
                {
                        LOG("--------------------S_SCALLBACK_REASON_CAMCONNECTED---------CIPLUS_APP_OPRF_RESOURCEID-------------------------");
                }
                d->ignore_close = 1;
                d->init_ok = 1;
                break;
        case S_SCALLBACK_REASON_CAMCONNECTFAIL: //2
                LOG("%02x:CAM on failed to connect to resource %08x", slot_id, resource_id);
                break;
        case S_SCALLBACK_REASON_CONNECTED: //3
                LOG("%02x:Host connection to resource %08x connected successfully, session_number %i",
                    slot_id, resource_id, session_number);
                break;
        case S_SCALLBACK_REASON_CONNECTFAIL: //4
                LOG("%02x:Host connection to resource %08x failed, session_number %i",
                    slot_id, resource_id, session_number);
                break;
        }
        return 0;
}

static int ca_lookup_callback(void *arg, uint8_t slot_id,
                              uint32_t requested_resource_id,
                              en50221_sl_resource_callback *callback_out, void **arg_out,
                              uint32_t *connected_resource_id)
{
        ca_device_t *d = arg;
        d->session->ca = d;

        LOG("===================> %s: slot_id %u requested_resource_id %x", __func__, slot_id,
            requested_resource_id);

        switch (requested_resource_id)
        {
        case EN50221_APP_RM_RESOURCEID:
        case TS101699_APP_RM_RESOURCEID:
                *callback_out = (en50221_sl_resource_callback)en50221_app_rm_message;
                *arg_out = d->rm_resource;
                *connected_resource_id = EN50221_APP_RM_RESOURCEID;
                break;
        case CIPLUS_APP_AI_RESOURCEID:
                *callback_out = (en50221_sl_resource_callback)ciplus_app_ai_message;
                *arg_out = d;
                *connected_resource_id = CIPLUS_APP_AI_RESOURCEID;
                break;
        case EN50221_APP_AI_RESOURCEID:
        case TS101699_APP_AI_RESOURCEID:
                *callback_out = (en50221_sl_resource_callback)en50221_app_ai_message;
                *arg_out = d->ai_resource;
                *connected_resource_id = requested_resource_id;
                break;
        case EN50221_APP_CA_RESOURCEID:
                *callback_out = (en50221_sl_resource_callback)en50221_app_ca_message;
                *arg_out = d->ca_resource;
                *connected_resource_id = EN50221_APP_CA_RESOURCEID;
                break;
        case EN50221_APP_DATETIME_RESOURCEID:
                *callback_out = (en50221_sl_resource_callback)en50221_app_datetime_message;
                *arg_out = d->dt_resource;
                *connected_resource_id = EN50221_APP_DATETIME_RESOURCEID;
                break;
        case EN50221_APP_MMI_RESOURCEID:
                *callback_out = (en50221_sl_resource_callback)en50221_app_mmi_message;
                *arg_out = d->mmi_resource;
                *connected_resource_id = EN50221_APP_MMI_RESOURCEID;
                break;
        case CIPLUS_APP_CC_RESOURCEID:
                /* CI Plus Implementation Guidelines V1.0.6 (2013-10)
   5.3.1 URI version advertisement
   A Host should advertise URI v1 only when Content Control v1 is selected by the CICAM */
                d->uri_mask = 0x1;
                *callback_out = (en50221_sl_resource_callback)ciplus_app_cc_message;
                *arg_out = d;
                *connected_resource_id = requested_resource_id;
                break;
        case CIPLUS_APP_CC_RESOURCEID_TWO:
        case CIPLUS_APP_CC_RESOURCEID_THREE:
        case CIPLUS_APP_CC_RESOURCEID_MULTI:
                d->uri_mask = 0x3;
                *callback_out = (en50221_sl_resource_callback)ciplus_app_cc_message;
                *arg_out = d;
                *connected_resource_id = requested_resource_id;
                break;
        case CIPLUS_APP_LANG_RESOURCEID:
                *callback_out = (en50221_sl_resource_callback)ciplus_app_lang_message;
                *arg_out = d;
                *connected_resource_id = requested_resource_id;
                break;
        case CIPLUS_APP_UPGR_RESOURCEID:
                *callback_out = (en50221_sl_resource_callback)ciplus_app_upgr_message;
                *arg_out = d;
                *connected_resource_id = requested_resource_id;
                break;
        case CIPLUS_APP_SAS_RESOURCEID:
                *callback_out = (en50221_sl_resource_callback)ciplus_app_sas_message;
                *arg_out = d;
                *connected_resource_id = requested_resource_id;
                break;
        case CIPLUS_APP_OPRF_RESOURCEID:
                *callback_out = (en50221_sl_resource_callback)ciplus_app_oprf_message;
                *arg_out = d;
                *connected_resource_id = requested_resource_id;
                break;
        default:
                *callback_out = (en50221_sl_resource_callback)en50221_app_unknown_message;
                *arg_out = d;
                *connected_resource_id = requested_resource_id;
                LOG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! lookup callback for unknown resource id %x on slot %u",
                    requested_resource_id, slot_id);
                break;
        }
        return 0;
}

static int ca_rm_enq_callback(void *arg, uint8_t slot_id,
                              uint16_t session_number)
{
        ca_device_t *d = arg;
        uint32_t *resource = resource_ids;
        uint32_t resource_count = resource_ids_count;
        int fd;
        fd = open("/etc/ssl/certs/root.pem", O_RDONLY);
        if (fd < 0)
                d->force_ci = 1;
        else
                close(fd);
        if (d->force_ci)
        {
                resource = resource_ids_ci;
                resource_count = resource_ids_ci_count;
        }

        LOG("%02x:%s  resource_count %i %s", slot_id, __func__, resource_count, d->force_ci ? "CI MODE" : "CI+ MODE");

        if (en50221_app_rm_reply(d->rm_resource, session_number, resource_count, resource))
        {
                LOG("%02x:Failed to send reply to ENQ", slot_id);
        }

        return 0;
}

static int ca_rm_reply_callback(void *arg, uint8_t slot_id,
                                uint16_t session_number, uint32_t resource_id_count,
                                uint32_t *_resource_ids)
{
        ca_device_t *d = arg;
        LOG("%02x:%s resource_count %i", slot_id, __func__, resource_id_count);

        uint32_t i;
        for (i = 0; i < resource_id_count; i++)
        {
                LOG("  CAM provided resource id: %08x", _resource_ids[i]);
        }

        if (en50221_app_rm_changed(d->rm_resource, session_number))
        {
                LOG("%02x:Failed to send REPLY", slot_id);
        }

        return 0;
}

static int ca_rm_changed_callback(void *arg, uint8_t slot_id,
                                  uint16_t session_number)
{
        ca_device_t *d = arg;
        LOG("%02x:%s", slot_id, __func__);

        if (en50221_app_rm_enq(d->rm_resource, session_number))
        {
                LOG("%02x:Failed to send ENQ", slot_id);
        }

        return 0;
}

static int ca_ca_info_callback(void *arg, uint8_t slot_id,
                               uint16_t session_number, uint32_t ca_id_count, uint16_t *ca_ids)
{
        (void)session_number;
        ca_device_t *d = arg;
        LOG("%02x:%s", slot_id, __func__);
        uint32_t i;
        for (i = 0; i < ca_id_count; i++)
        {
                LOG("  Supported CA ID: %04x for CA%d", ca_ids[i], d->id);
                add_caid_mask(dvbca_id, d->id, ca_ids[i], 0xFFFF);
        }

        return 0;
}

static int ca_ca_pmt_reply_callback(void *arg, uint8_t slot_id,
                                    uint16_t session_number, struct en50221_app_pmt_reply *reply,
                                    uint32_t reply_size)
{
        (void)arg;
        (void)session_number;
        (void)reply;
        (void)reply_size;

        LOG("ca_ca_pmt_reply_callback %02x:%s", slot_id, __func__);

        if (!reply->CA_enable_flag || reply->CA_enable != 1)
        {
                LOG("Unable to descramble. ca_enable_flag: %d, ca_enable: 0x%02x", reply->CA_enable_flag, reply->CA_enable);
                return 0;
        }

        LOG("OK descrambling");

        return 0;
}

static int ca_dt_enquiry_callback(void *arg, uint8_t slot_id,
                                  uint16_t session_number, uint8_t response_interval)
{
        ca_device_t *d = arg;

        LOG("%02x:%s", slot_id, __func__);
        LOG("  response_interval:%i", response_interval);

        if (en50221_app_datetime_send(d->dt_resource, session_number, time(NULL),
                                      -1))
        {
                LOG("%02x:Failed to send datetime", slot_id);
        }

        return 0;
}

static int
ca_mmi_close_callback(void *arg, uint8_t slot_id, uint16_t session_number,
                      uint8_t cmd_id, uint8_t delay)
{
        LOG("mmi close cb received for slot %u session_num %u "
            "cmd_id 0x%02x delay %u",
            slot_id, session_number, cmd_id, delay);

        return 0;
}

static int
ca_mmi_display_ctl_callback(void *arg, uint8_t slot_id, uint16_t session_number,
                            uint8_t cmd_id, uint8_t mmi_mode)
{
        ca_device_t *d = arg;

        LOG("mmi display ctl cb received for slot %u session_num %u "
            "cmd_id 0x%02x mmi_mode %u",
            slot_id, session_number, cmd_id, mmi_mode);

        if (cmd_id == MMI_DISPLAY_CONTROL_CMD_ID_SET_MMI_MODE)
        {
                struct en50221_app_mmi_display_reply_details det;

                det.u.mode_ack.mmi_mode = mmi_mode;
                if (en50221_app_mmi_display_reply(d->mmi_resource, session_number,
                                                  MMI_DISPLAY_REPLY_ID_MMI_MODE_ACK, &det))
                {
                        LOG("Slot %u: Failed to send MMI mode ack reply", slot_id);
                }
        }

        return 0;
}

static int
ca_mmi_enq_callback(void *arg, uint8_t slot_id, uint16_t session_number,
                    uint8_t blind_answ, uint8_t exp_answ_len,
                    uint8_t *text, uint32_t text_size)
{
        ca_device_t *d = arg;
        char buffer[256];

        snprintf(buffer, sizeof(buffer), "%.*s", text_size, text);

        LOG("MMI enquiry from CAM in slot %u:  %s (%s%u digits)",
            slot_id, buffer, blind_answ ? "blind " : "", exp_answ_len);

        if (strlen((char *)d->pin_str) == exp_answ_len)
        {
                LOG("answering to PIN enquiry");
                en50221_app_mmi_answ(d->mmi_resource, session_number,
                                     MMI_ANSW_ID_ANSWER, (uint8_t *)d->pin_str,
                                     exp_answ_len);
        }

        en50221_app_mmi_close(d->mmi_resource, session_number,
                              MMI_CLOSE_MMI_CMD_ID_IMMEDIATE, 0);

        return 0;
}

static int
ca_mmi_menu_callback(void *arg, uint8_t slot_id, uint16_t session_number,
                     struct en50221_app_mmi_text *title,
                     struct en50221_app_mmi_text *sub_title,
                     struct en50221_app_mmi_text *bottom,
                     uint32_t item_count, struct en50221_app_mmi_text *items,
                     uint32_t item_raw_length, uint8_t *items_raw)
{
        ca_device_t *d = arg;

        LOG("MMI menu from CAM in the slot %u:", slot_id);
        LOG("  title:    %.*s", title->text_length, title->text);
        LOG("  subtitle: %.*s", sub_title->text_length, sub_title->text);

        uint32_t i;
        for (i = 0; i < item_count; i++)
        {
                LOG("  item %i:   %.*s", i + 1, items[i].text_length, items[i].text);
        }
        LOG("  bottom:   %.*s", bottom->text_length, bottom->text);

        /* menu answer OK */
        en50221_app_mmi_menu_answ(d->mmi_resource, session_number,
                                  0x01);
        /* enter "OK" */
        //        en50221_app_mmi_keypress(d->mmi_resource, session_number,
        //                                  0x11);
        /* cancel menu */
        en50221_app_mmi_close(d->mmi_resource, session_number,
                              MMI_CLOSE_MMI_CMD_ID_IMMEDIATE, 0);

        return 0;
}

static int
ca_app_mmi_list_callback(void *arg, uint8_t slot_id, uint16_t session_num,
                         struct en50221_app_mmi_text *title,
                         struct en50221_app_mmi_text *sub_title,
                         struct en50221_app_mmi_text *bottom,
                         uint32_t item_count, struct en50221_app_mmi_text *items,
                         uint32_t item_raw_length, uint8_t *items_raw)
{
        ca_device_t *d = arg;

        LOG("MMI list from CAM in the slot %u:", slot_id);
        LOG("  title:    %.*s", title->text_length, title->text);
        LOG("  subtitle: %.*s", sub_title->text_length, sub_title->text);

        uint32_t i;
        for (i = 0; i < item_count; i++)
        {
                LOG("  item %i:   %.*s", i + 1, items[i].text_length, items[i].text);
        }
        LOG("  bottom:   %.*s", bottom->text_length, bottom->text);

        /* cancel menu */
        en50221_app_mmi_close(d->mmi_resource, session_num,
                              MMI_CLOSE_MMI_CMD_ID_IMMEDIATE, 0);
        return 0;
}

static int
ca_mmi_keypad_control_callback(void *arg, uint8_t slot_id, uint16_t session_number,
                               uint8_t cmd_id, uint8_t *key_codes, uint32_t key_codes_count)
{
        (void)arg;
        (void)session_number;
        (void)cmd_id;
        (void)key_codes;
        (void)key_codes_count;

        LOG("================================  %02x:%s", slot_id, __func__);

        return 0;
}

static int
ca_mmi_subtitle_segment_callback(void *arg, uint8_t slot_id, uint16_t session_number,
                                 uint8_t *segment, uint32_t segment_size)
{
        (void)arg;
        (void)session_number;
        (void)segment;
        (void)segment_size;

        LOG("%02x:%s", slot_id, __func__);

        return 0;
}

static int
ca_mmi_scene_end_mark_callback(void *arg, uint8_t slot_id, uint16_t session_number,
                               uint8_t decoder_continue_flag, uint8_t scene_reveal_flag,
                               uint8_t send_scene_done, uint8_t scene_tag)
{
        (void)arg;
        (void)session_number;
        (void)decoder_continue_flag;
        (void)scene_reveal_flag;
        (void)send_scene_done;
        (void)scene_tag;

        LOG("%02x:%s", slot_id, __func__);

        return 0;
}

static int
ca_mmi_scene_control_callback(void *arg, uint8_t slot_id, uint16_t session_number,
                              uint8_t decoder_continue_flag, uint8_t scene_reveal_flag,
                              uint8_t scene_tag)
{
        (void)arg;
        (void)session_number;
        (void)decoder_continue_flag;
        (void)scene_reveal_flag;
        (void)scene_tag;

        LOG("%02x:%s", slot_id, __func__);

        return 0;
}

static int
ca_mmi_subtitle_download_callback(void *arg, uint8_t slot_id, uint16_t session_number,
                                  uint8_t *segment, uint32_t segment_size)
{
        (void)arg;
        (void)session_number;
        (void)segment;
        (void)segment_size;

        LOG("%02x:%s", slot_id, __func__);

        return 0;
}

static int
ca_mmi_flush_download_callback(void *arg, uint8_t slot_id, uint16_t session_number)
{
        (void)arg;
        (void)session_number;

        LOG("%02x:%s", slot_id, __func__);

        return 0;
}

int ca_init(ca_device_t *d)
{
        ca_slot_info_t info;
        int64_t st = getTick();
        __attribute__((unused)) int tries = 800; // wait up to 8s for the CAM
        int fd = d->fd;
        d->tl = NULL;
        d->sl = NULL;
        d->slot_id = -1;
        memset(&info, 0, sizeof(info));

        if (ioctl(fd, CA_RESET, &info))
                LOG_AND_RETURN(0, "%s: Could not reset ca %d", __FUNCTION__, d->id);
        d->sp = 0;
        d->is_ciplus = 0;
        do
        {
                if (ioctl(fd, CA_GET_SLOT_INFO, &info))
                        LOG_AND_RETURN(0, "%s: Could not get info1 for ca %d", __FUNCTION__, d->id);
                usleep(10000);
        } while ((tries-- > 0) && !(info.flags & CA_CI_MODULE_READY));

        if (ioctl(fd, CA_GET_SLOT_INFO, &info))
                LOG_AND_RETURN(0, "%s: Could not get info2 for ca %d, tries %d", __FUNCTION__, d->id, tries);

        LOG("initializing CA, fd %d type %d flags 0x%x, after %jd ms", fd, info.type, info.flags, (getTick() - st));

        if (info.type != CA_CI_LINK)
        {
                LOG("incompatible CA interface");
                goto fail;
        }

        if (!(info.flags & CA_CI_MODULE_READY))
        {
                LOG("CA module not present or not ready");
                goto fail;
        }

        if ((d->tl = en50221_tl_create(8, 32)) == NULL)
        {
                LOG("failed to create transport layer");
                goto fail;
        }

        if ((d->slot_id = en50221_tl_register_slot(d->tl, fd, 0, 10000, 1000)) < 0)
        {
                LOG("slot registration failed");
                goto fail;
        }
        LOG("slotid: %i", d->slot_id);

        // create session layer
        d->sl = en50221_sl_create(d->tl, 256);
        if (d->sl == NULL)
        {
                LOG("failed to create session layer");
                goto fail;
        }

        // create the sendfuncs
        d->sf.arg = d->sl;
        d->sf.send_data = (en50221_send_data)en50221_sl_send_data;
        d->sf.send_datav = (en50221_send_datav)en50221_sl_send_datav;

        /* create app resources and assign callbacks */
        d->rm_resource = en50221_app_rm_create(&d->sf);
        en50221_app_rm_register_enq_callback(d->rm_resource, ca_rm_enq_callback, d);
        en50221_app_rm_register_reply_callback(d->rm_resource, ca_rm_reply_callback, d);
        en50221_app_rm_register_changed_callback(d->rm_resource, ca_rm_changed_callback, d);

        d->dt_resource = en50221_app_datetime_create(&d->sf);
        en50221_app_datetime_register_enquiry_callback(d->dt_resource, ca_dt_enquiry_callback, d);

        d->ai_resource = en50221_app_ai_create(&d->sf);
        en50221_app_ai_register_callback(d->ai_resource, ca_ai_callback, d);

        d->ca_resource = en50221_app_ca_create(&d->sf);
        en50221_app_ca_register_info_callback(d->ca_resource, ca_ca_info_callback, d);
        en50221_app_ca_register_pmt_reply_callback(d->ca_resource, ca_ca_pmt_reply_callback, d);

        d->mmi_resource = en50221_app_mmi_create(&d->sf);
        en50221_app_mmi_register_close_callback(d->mmi_resource, ca_mmi_close_callback, d);
        en50221_app_mmi_register_display_control_callback(d->mmi_resource, ca_mmi_display_ctl_callback, d);
        en50221_app_mmi_register_keypad_control_callback(d->mmi_resource, ca_mmi_keypad_control_callback, d);

        en50221_app_mmi_register_subtitle_segment_callback(d->mmi_resource, ca_mmi_subtitle_segment_callback, d);
        en50221_app_mmi_register_scene_end_mark_callback(d->mmi_resource, ca_mmi_scene_end_mark_callback, d);
        en50221_app_mmi_register_scene_control_callback(d->mmi_resource, ca_mmi_scene_control_callback, d);
        en50221_app_mmi_register_subtitle_download_callback(d->mmi_resource, ca_mmi_subtitle_download_callback, d);
        en50221_app_mmi_register_flush_download_callback(d->mmi_resource, ca_mmi_flush_download_callback, d);

        en50221_app_mmi_register_enq_callback(d->mmi_resource, ca_mmi_enq_callback, d);
        en50221_app_mmi_register_menu_callback(d->mmi_resource, ca_mmi_menu_callback, d);
        en50221_app_mmi_register_list_callback(d->mmi_resource, ca_app_mmi_list_callback, d);

        pthread_create(&d->stackthread, NULL, stackthread_func, d);

        en50221_sl_register_lookup_callback(d->sl, ca_lookup_callback, d);
        en50221_sl_register_session_callback(d->sl, ca_session_callback, d);

        d->tc = en50221_tl_new_tc(d->tl, d->slot_id);
        LOG("tcid: %i", d->tc);

        return 0;
fail:
        close(fd);
        d->fd = -1;
        d->enabled = 0;
        return 1;
}

ca_device_t *alloc_ca_device()
{
        ca_device_t *d = malloc1(sizeof(ca_device_t));
        if (!d)
        {
                LOG_AND_RETURN(NULL, "Could not allocate memory for CA device");
        }
        memset(d, 0, sizeof(ca_device_t));
        return d;
}

int dvbca_init_dev(adapter *ad)
{
        ca_device_t *c = ca_devices[ad->id];
        int fd;
        char ca_dev_path[100];

        if (c && c->enabled)
                return TABLES_RESULT_OK;

        if (ad->type != ADAPTER_DVB)
                return TABLES_RESULT_ERROR_NORETRY;
#ifdef ENIGMA
        sprintf(ca_dev_path, "/dev/ci%d", ad->pa);
#else
        sprintf(ca_dev_path, "/dev/dvb/adapter%d/ca0", ad->pa);
#endif
        fd = open(ca_dev_path, O_RDWR);
        if (fd < 0)
                LOG_AND_RETURN(TABLES_RESULT_ERROR_NORETRY, "No CA device detected on adapter %d: file %s", ad->id, ca_dev_path);
        if (!c)
        {
                c = ca_devices[ad->id] = alloc_ca_device();
                if (!c)
                {
                        close(fd);
                        LOG_AND_RETURN(0, "Could not allocate memory for CA device %d", ad->id);
                }
        }
        c->enabled = 1;
        c->ignore_close = 0;
        c->fd = fd;
        c->id = ad->id;
        c->ca_high_bitrate_mode = 0;
        c->stackthread = 0;
        c->init_ok = 0;
        memset(c->pmt_id, -1, sizeof(c->pmt_id));
        memset(c->key[0], 0, sizeof(c->key[0]));
        memset(c->key[1], 0, sizeof(c->key[1]));
        memset(c->iv[0], 0, sizeof(c->iv[0]));
        memset(c->iv[1], 0, sizeof(c->iv[1]));
        if (ca_init(c))
        {
                dvbca_close_device(c);
                return TABLES_RESULT_ERROR_NORETRY;
        }
        return TABLES_RESULT_OK;
}

int dvbca_close_device(ca_device_t *c)
{
        LOG("closing CA device %d, fd %d", c->id, c->fd);
        c->enabled = 0;
        if (c->stackthread)
                pthread_join(c->stackthread, NULL);
        if (c->tl && (c->slot_id >= 0))
                en50221_tl_destroy_slot(c->tl, c->slot_id);
        if (c->sl)
                en50221_sl_destroy(c->sl);
        if (c->tl)
                en50221_tl_destroy(c->tl);
        if (c->fd >= 0)
                close(c->fd);
        EVP_cleanup();
        ERR_free_strings();
        return 0;
}
int dvbca_close_dev(adapter *ad)
{
        ca_device_t *c = ca_devices[ad->id];
        if (c && c->enabled && !c->ignore_close) // do not close the CA unless in a bad state
        {
                dvbca_close_device(c);
        }
        return 1;
}

int dvbca_close()
{
        int i;
        for (i = 0; i < MAX_ADAPTERS; i++)
                if (ca_devices[i] && ca_devices[i]->enabled)
                {
                        dvbca_close_device(ca_devices[i]);
                }
        return 0;
}

SCA_op dvbca;

void dvbca_init() // you can search the devices here and fill the ca_devices, then open them here (for example independent CA devices), or use dvbca_init_dev to open them (like in this module)
{
        memset(&dvbca, 0, sizeof(dvbca));
        dvbca.ca_init_dev = dvbca_init_dev;
        dvbca.ca_close_dev = dvbca_close_dev;
        dvbca.ca_add_pmt = dvbca_process_pmt;
        dvbca.ca_del_pmt = dvbca_del_pmt;
        dvbca.ca_close_ca = dvbca_close;
        dvbca.ca_ts = NULL; //dvbca_ts;
        dvbca_id = add_ca(&dvbca, 0xFFFFFFFF);
}

char *get_ca_pin(int i)
{
        if (ca_devices[i])
                return ca_devices[i]->pin_str;
        return NULL;
}

void set_ca_pin(int i, char *pin)
{
        if (!ca_devices[i])
                ca_devices[i] = alloc_ca_device();
        if (!ca_devices[i])
                return;
        memset(ca_devices[i]->pin_str, 0, sizeof(ca_devices[i]->pin_str));
        strncpy(ca_devices[i]->pin_str, pin, sizeof(ca_devices[i]->pin_str) - 1);
}

void force_ci_adapter(int i)
{
        if (!ca_devices[i])
                ca_devices[i] = alloc_ca_device();
        if (!ca_devices[i])
                return;
        ca_devices[i]->force_ci = 1;
}

void set_ca_adapter_force_ci(char *o)
{
        int i, j, la, st, end;
        char buf[1000], *arg[40], *sep;
        SAFE_STRCPY(buf, o);
        la = split(arg, buf, ARRAY_SIZE(arg), ',');
        for (i = 0; i < la; i++)
        {
                sep = strchr(arg[i], '-');

                if (sep == NULL)
                {
                        st = end = map_int(arg[i], NULL);
                }
                else
                {
                        st = map_int(arg[i], NULL);
                        end = map_int(sep + 1, NULL);
                }
                for (j = st; j <= end; j++)
                {

                        force_ci_adapter(j);
                        LOG("Forcing CA %d to CI", j);
                }
        }
}

void set_ca_adapter_pin(char *o)
{
        int i, j, la, st, end;
        char buf[1000], *arg[40], *sep, *seps;
        SAFE_STRCPY(buf, o);
        la = split(arg, buf, ARRAY_SIZE(arg), ',');
        for (i = 0; i < la; i++)
        {
                sep = strchr(arg[i], '-');
                seps = strchr(arg[i], ':');

                if (!seps)
                        continue;

                if (sep == NULL)
                {
                        st = end = map_int(arg[i], NULL);
                }
                else
                {
                        st = map_int(arg[i], NULL);
                        end = map_int(sep + 1, NULL);
                }
                for (j = st; j <= end; j++)
                {
                        set_ca_pin(j, seps + 1);
                        LOG("Setting CA %d pin to %s", j, seps + 1);
                }
        }
}
