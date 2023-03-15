#ifndef CA_H
#define CA_H
#include "adapter.h"
#include "aes.h"
#include "pmt.h"
#define MAX_CA_PMT 4
#define DEFAULT_CA_PMT 4
#define MAX_SESSIONS 64
#define PMT_INVALID -1
#define PMT_ID_IS_VALID(x) (x > PMT_INVALID)
#define MAX_ELEMENTS 33
#define MAX_PAIRS 10

#define SIZE_INDICATOR 0x80
#define T_SB 0x80
#define T_RCV 0x81
#define T_CREATE_TC 0x82
#define T_CTC_REPLY 0x83
#define T_DELETE_TC 0x84
#define T_DTC_REPLY 0x85
#define T_REQUEST_TC 0x86
#define T_NEW_TC 0x87
#define T_TC_ERROR 0x88
#define T_DATA_LAST 0xA0
#define T_DATA_MORE 0xA1

/*
 * Session layer
 */

#define ST_SESSION_NUMBER 0x90
#define ST_OPEN_SESSION_REQUEST 0x91
#define ST_OPEN_SESSION_RESPONSE 0x92
#define ST_CREATE_SESSION 0x93
#define ST_CREATE_SESSION_RESPONSE 0x94
#define ST_CLOSE_SESSION_REQUEST 0x95
#define ST_CLOSE_SESSION_RESPONSE 0x96

/*
 * Application layer
 */

// RM
#define TAG_PROFILE_ENQUIRY 0x9F8010
#define TAG_PROFILE 0x9F8011
#define TAG_PROFILE_CHANGE 0x9F8012

// AI
#define TAG_APP_INFO_ENQUIRY 0x9f8020
#define CI_DATA_RATE_INFO 0x9f8024

// CA
#define TAG_CA_INFO_ENQUIRY 0x9f8030
#define TAG_CA_INFO 0x9F8031
#define TAG_CA_PMT 0x9f8032
#define CA_PMT_CMD_ID_OK_DESCRAMBLING 0x01

// Date Time
#define TAG_DATE_TIME_ENQUIRY 0x9f8440
#define TAG_DATE_TIME 0x9f8441

// MMI
#define TAG_CLOSE_MMI 0x9f8800
#define TAG_DISPLAY_CONTROL 0x9f8801
#define TAG_DISPLAY_REPLY 0x9f8802
#define TAG_ENQUIRY 0x9f8807
#define TAG_ANSWER 0x9f8808
#define TAG_MENU_LAST 0x9f8809
#define TAG_MENU_MORE 0x9f880a
#define TAG_MENU_ANSWER 0x9f880b
#define TAG_LIST_LAST 0x9f880c
#define TAG_LIST_MORE 0x9f880d

#define MMI_CLOSE_MMI_CMD_ID_IMMEDIATE 0x00
#define MMI_ANSW_ID_CANCEL 0x00
#define MMI_ANSW_ID_ANSWER 0x01
#define MMI_DISPLAY_REPLY_ID_MMI_MODE_ACK 0x01
#define MMI_DISPLAY_CONTROL_CMD_ID_SET_MMI_MODE 0x01

// CI+
#define CIPLUS_TAG_CC_OPEN_REQ 0x9f9001
#define CIPLUS_TAG_CC_OPEN_CNF 0x9f9002
#define CIPLUS_TAG_CC_DATA_REQ 0x9f9003
#define CIPLUS_TAG_CC_DATA_CNF 0x9f9004
#define CIPLUS_TAG_CC_SYNC_REQ 0x9f9005
#define CIPLUS_TAG_CC_SYNC_CNF 0x9F9006
#define CIPLUS_TAG_CC_SAC_DATA_REQ 0x9f9007
#define CIPLUS_TAG_CC_SAC_DATA_CNF 0x9F9008
#define CIPLUS_TAG_CC_SAC_SYNC_REQ 0x9f9009
#define CIPLUS_TAG_CC_SAC_SYNC_CNF 0x9F9010

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

#define MAKE_SID_FOR_CA(id, idx) (id * 100 + idx)
#define MKRID(CLASS, TYPE, VERSION)                                            \
    ((((CLASS)&0xffff) << 16) | (((TYPE)&0x3ff) << 6) | ((VERSION)&0x3f))

#define CA_STATE_INACTIVE 0
#define CA_STATE_ACTIVE 1
#define CA_STATE_INITIALIZED 2

// Contains informations needed to send a CAPMT
// If multiple_pmt is 0, other_id will be PMT_INVALID
typedef struct ca_pmt {
    int pmt_id;
    int other_id;
    int version;
    int sid;
} SCAPMT;

extern char *listmgmt_str[];
typedef struct ca_device ca_device_t;
typedef struct ca_session ca_session_t;

struct struct_application_handler {
    int resource;
    char *name;
    int (*callback)(ca_session_t *session, int resource, uint8_t *buffer,
                    int len);
    int (*create)(ca_session_t *session, int resource);
    int (*close)(ca_session_t *session);
};

struct ca_session {
    struct struct_application_handler handler;
    ca_device_t *ca;
    int session_number;
    int ai_version;
};

struct element {
    uint8_t *data;
    uint32_t size;
    /* buffer valid */
    int valid;
};

struct cc_ctrl_data {
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

struct ca_device {
    int enabled;
    SCAPMT capmt[MAX_CA_PMT];
    int max_ca_pmt, multiple_pmt;
    int fd, sock;
    int id;
    int state;
    uint16_t caid[MAX_CAID];
    uint32_t caids;
    int has_forced_caids;

    struct cc_ctrl_data private_data;
    ca_session_t sessions[MAX_SESSIONS];

    int uri_mask;
    char force_ci;
    char linked_adapter;

    /*
     * CAM module info
     */
    char ci_name[128];

    char cam_menu_string[64];
    char pin_str[10];
    uint8_t key[2][16], iv[2][16];
    int sp, parity;

    /*
     * CAM date time handling
     */
    uint64_t datetime_response_interval;
    uint64_t datetime_next_send;
};

extern ca_device_t *ca_devices[];

ca_device_t *alloc_ca_device();
int ca_init(ca_device_t *d);
void dvbca_init();
int create_capmt(SCAPMT *ca, int listmgmt, uint8_t *capmt, int capmt_len,
                 int cmd_id, int added_only);
int is_ca_initializing(int i);
void set_ca_adapter_pin(char *o);
void set_ca_adapter_force_ci(char *o);
char *get_ca_pin(int i);
void set_ca_channels(char *o);
int get_ca_multiple_pmt(int i);
int get_max_pmt_for_ca(int i);
void get_authdata_filename(char *dest, size_t len, unsigned int slot,
                           char *ci_name);
char *get_ca_caids_string(int i, char *dest, int max_len);

#endif
