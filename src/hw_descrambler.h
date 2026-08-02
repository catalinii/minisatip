#ifndef HW_DESCRAMBLER_H
#define HW_DESCRAMBLER_H

#include "adapter.h"
#include "opts.h"
#include "pmt.h"

#include <array>
#include <cstdint>
#include <linux/dvb/ca.h>
#include <sys/ioctl.h>

#ifndef CA_SET_PID
#define CA_SET_PID _IOW('o', 135, struct ca_pid)
struct ca_pid {
    unsigned int pid;
    int index;
};
#endif

#ifndef CA_GET_DESCR_INFO
#define CA_GET_DESCR_INFO _IOR('o', 133, struct ca_descr_info)
struct ca_descr_info {
    unsigned int num;  // Total available hardware descrambler slots
    unsigned int type; // CA type bitmask
};
#endif

// Extended Enigma2 DVB CA ioctl definitions
#ifndef CA_SET_DESCR_MODE
#define CA_SET_DESCR_MODE _IOW('o', 136, struct ca_descr_mode)
struct ca_descr_mode {
    unsigned int index;
    unsigned int algo;        // 0: DVBCSA, 1: DES, 2: AES128-ECB, 3: AES128-CBC
    unsigned int cipher_mode; // 0: ECB, 1: CBC
};
#endif

#ifndef CA_SET_DESCR_DATA
#define CA_SET_DESCR_DATA _IOW('o', 137, struct ca_descr_data)
struct ca_descr_data {
    unsigned int index;
    unsigned int parity;    // 0: EVEN, 1: ODD
    unsigned int data_type; // 0: CW, 1: IV
    unsigned int data_len;  // Length (16 bytes for 128-bit key/IV)
    unsigned char data[16];
};
#endif

void init_hw_descrambler();

#endif // HW_DESCRAMBLER_H
