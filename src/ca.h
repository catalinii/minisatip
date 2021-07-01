#ifndef CA_H
#define CA_H
#include "adapter.h"
#include "tables.h"
#define MAX_CA_PMT 4
#define PMT_INVALID -1
#define PMT_ID_IS_VALID(x) (x > PMT_INVALID)

#define MAKE_SID_FOR_CA(id, idx) (id * 100 + idx)
// Contains informations needed to send a CAPMT
// If multiple_pmt is 0, other_id will be PMT_INVALID
typedef struct ca_pmt {
    int pmt_id;
    int other_id;
    int version;
    int sid;
} SCAPMT;

int ca_init(ca_device_t *d);
void dvbca_init();
int create_capmt(SCAPMT *ca, int listmgmt, uint8_t *capmt, int capmt_len,
                 int cmd_id);
int is_ca_initializing(int i);
void set_ca_adapter_pin(char *o);
void set_ca_adapter_force_ci(char *o);
char *get_ca_pin(int i);
void set_ca_multiple_pmt(char *o);
int get_max_pmt_for_ca(int i);
#endif
