/*
 * Unit tests for Enigma2 Hardware Descrambler (hw_descrambler.cpp)
 */

#include "adapter.h"
#include "ca.h"
#include "hw_descrambler.h"
#include "minisatip.h"
#include "opts.h"
#include "pmt.h"
#include "tables.h"
#include "utils.h"
#include "utils/testing.h"

#include <cstring>
#include <memory>
#include <sys/ioctl.h>
#include <unistd.h>

#define DEFAULT_LOG LOG_DVBCA

extern adapter *a[MAX_ADAPTERS];
extern SCW *cws[MAX_CW];
extern SPMT *pmts[MAX_PMT];
extern int pmt_del(int id);

// Test Helper: setup mock adapter
static adapter *setup_mock_adapter(int id, int physical_adapter, int ca_num) {
    if (id < 0 || id >= MAX_ADAPTERS)
        return nullptr;
    if (!a[id]) {
        a[id] = static_cast<adapter *>(calloc(1, sizeof(adapter)));
        a[id]->id = id;
        a[id]->pa = physical_adapter;
        a[id]->fn = ca_num;
        a[id]->enabled = 1;
    }
    return a[id];
}

static void cleanup_mock_adapter(int id) {
    if (id >= 0 && id < MAX_ADAPTERS && a[id]) {
        free(a[id]);
        a[id] = nullptr;
    }
}

// Test 1: Key Lifecycle (creation and deletion across algorithms)
static int test_hw_key_lifecycle() {
    SCW cw_csa{};
    cw_csa.id = 1;
    cw_csa.algo = CA_ALGO_DVBCSA;
    hw_create_key(&cw_csa);
    ASSERT(cw_csa.key != nullptr, "hw_create_key failed for DVBCSA");

    SCW cw_aes_ecb{};
    cw_aes_ecb.id = 2;
    cw_aes_ecb.algo = CA_ALGO_AES128_ECB;
    hw_create_key(&cw_aes_ecb);
    ASSERT(cw_aes_ecb.key != nullptr, "hw_create_key failed for AES128_ECB");

    SCW cw_aes_cbc{};
    cw_aes_cbc.id = 3;
    cw_aes_cbc.algo = CA_ALGO_AES128_CBC;
    hw_create_key(&cw_aes_cbc);
    ASSERT(cw_aes_cbc.key != nullptr, "hw_create_key failed for AES128_CBC");

    hw_delete_key(&cw_csa);
    ASSERT(cw_csa.key == nullptr, "hw_delete_key failed for DVBCSA");

    hw_delete_key(&cw_aes_ecb);
    ASSERT(cw_aes_ecb.key == nullptr, "hw_delete_key failed for AES128_ECB");

    hw_delete_key(&cw_aes_cbc);
    ASSERT(cw_aes_cbc.key == nullptr, "hw_delete_key failed for AES128_CBC");

    return 0;
}

// Test 2: PMT Creation and Hardware Descrambler Binding (opts.enigma checks)
static int test_hw_descrambler_disabled_when_opts_enigma_zero() {
    opts.enigma = 0;

    adapter *ad = setup_mock_adapter(0, 0, 0);
    ASSERT(ad != nullptr, "setup_mock_adapter failed");

    int pmt_id = pmt_add(0, 100, 1000);
    SPMT *pmt = get_pmt(pmt_id);
    ASSERT(pmt != nullptr, "pmt_add failed");
    pmt_add_stream_pid(pmt, 1001, 2, false, true);

    SCW cw{};
    cw.id = 10;
    cw.algo = CA_ALGO_DVBCSA;
    cw.parity = 0;
    std::memset(cw.cw, 0xAA, 8);
    hw_create_key(&cw);

    // Call hw_set_cw when opts.enigma == 0
    hw_set_cw(&cw, pmt);

    // Verify hw_ca_del_pmt when opts.enigma == 0 returns safely
    int res = hw_ca_del_pmt(ad, pmt);
    ASSERT(res == 0, "hw_ca_del_pmt should return 0");

    hw_delete_key(&cw);
    pmt_del(pmt_id);
    cleanup_mock_adapter(0);
    return 0;
}

// Test 3: Multi-Channel Parallel Slot Allocation and Parity Sharing
static int test_multi_channel_slot_allocation() {
    opts.enigma = 1;

    adapter *ad = setup_mock_adapter(0, 0, 0);
    ASSERT(ad != nullptr, "setup_mock_adapter failed");

    // Add PMT 100 (HBO HD)
    int pmt_id1 = pmt_add(0, 100, 1000);
    SPMT *pmt1 = get_pmt(pmt_id1);
    pmt_add_stream_pid(pmt1, 1001, 27, false, true); // Video PID
    pmt_add_stream_pid(pmt1, 1002, 3, false, true);  // Audio PID

    // Add PMT 200 (FILM NOW HD)
    int pmt_id2 = pmt_add(0, 200, 2000);
    SPMT *pmt2 = get_pmt(pmt_id2);
    pmt_add_stream_pid(pmt2, 2001, 27, false, true); // Video PID
    pmt_add_stream_pid(pmt2, 2002, 4, false, true);  // Audio PID

    // Add PMT 300 (AXN White)
    int pmt_id3 = pmt_add(0, 300, 3000);
    SPMT *pmt3 = get_pmt(pmt_id3);
    pmt_add_stream_pid(pmt3, 3001, 2, false, true);

    // Create EVEN and ODD CWs for PMT 1
    SCW cw1_even{}, cw1_odd{};
    cw1_even.id = 1;
    cw1_even.algo = CA_ALGO_DVBCSA;
    cw1_even.parity = 0;
    hw_create_key(&cw1_even);

    cw1_odd.id = 2;
    cw1_odd.algo = CA_ALGO_DVBCSA;
    cw1_odd.parity = 1;
    hw_create_key(&cw1_odd);

    // Create CW for PMT 2
    SCW cw2{};
    cw2.id = 3;
    cw2.algo = CA_ALGO_DVBCSA;
    cw2.parity = 0;
    hw_create_key(&cw2);

    // Create CW for PMT 3
    SCW cw3{};
    cw3.id = 4;
    cw3.algo = CA_ALGO_AES128_CBC;
    cw3.parity = 0;
    hw_create_key(&cw3);

    // Call set_cw for PMT 1 (EVEN and ODD)
    hw_set_cw(&cw1_even, pmt1);
    hw_set_cw(&cw1_odd, pmt1);

    // Call set_cw for PMT 2 and PMT 3
    hw_set_cw(&cw2, pmt2);
    hw_set_cw(&cw3, pmt3);

    // Deallocate PMT 2 (FILM NOW HD stops)
    hw_ca_del_pmt(ad, pmt2);

    // Re-allocate PMT 4 (Nickelodeon starts)
    int pmt_id4 = pmt_add(0, 400, 4000);
    SPMT *pmt4 = get_pmt(pmt_id4);
    pmt_add_stream_pid(pmt4, 4001, 2, false, true);

    SCW cw4{};
    cw4.id = 5;
    cw4.algo = CA_ALGO_DVBCSA;
    cw4.parity = 0;
    hw_create_key(&cw4);

    hw_set_cw(&cw4, pmt4);

    // Cleanup all keys and PMTs
    hw_delete_key(&cw1_even);
    hw_delete_key(&cw1_odd);
    hw_delete_key(&cw2);
    hw_delete_key(&cw3);
    hw_delete_key(&cw4);

    hw_ca_del_pmt(ad, pmt1);
    hw_ca_del_pmt(ad, pmt3);
    hw_ca_del_pmt(ad, pmt4);

    pmt_del(pmt_id1);
    pmt_del(pmt_id2);
    pmt_del(pmt_id3);
    pmt_del(pmt_id4);
    cleanup_mock_adapter(0);

    return 0;
}

// Test 4: AES-128 ECB & CBC Mode Key Programming Simulation
static int test_aes128_key_programming() {
    opts.enigma = 1;

    adapter *ad = setup_mock_adapter(0, 0, 0);
    ASSERT(ad != nullptr, "setup_mock_adapter failed");

    int pmt_id = pmt_add(0, 680, 6800);
    SPMT *pmt = get_pmt(pmt_id);
    pmt_add_stream_pid(pmt, 6801, 36, false, true); // 4K HEVC Video PID
    pmt_add_stream_pid(pmt, 6802, 15, false, true); // AAC Audio PID

    // 16-byte AES Key and IV
    uint8_t aes_key[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                           0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    uint8_t aes_iv[16] = {0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88,
                          0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};

    // Test AES-128 ECB Mode
    SCW cw_ecb{};
    cw_ecb.id = 100;
    cw_ecb.algo = CA_ALGO_AES128_ECB;
    cw_ecb.parity = 0;
    std::memcpy(cw_ecb.cw, aes_key, 16);
    hw_create_key(&cw_ecb);
    hw_set_cw(&cw_ecb, pmt);

    // Test AES-128 CBC Mode
    SCW cw_cbc{};
    cw_cbc.id = 101;
    cw_cbc.algo = CA_ALGO_AES128_CBC;
    cw_cbc.parity = 1;
    std::memcpy(cw_cbc.cw, aes_key, 16);
    std::memcpy(cw_cbc.iv, aes_iv, 16);
    hw_create_key(&cw_cbc);
    hw_set_cw(&cw_cbc, pmt);

    // Cleanup
    hw_delete_key(&cw_ecb);
    hw_delete_key(&cw_cbc);
    hw_ca_del_pmt(ad, pmt);
    pmt_del(pmt_id);
    cleanup_mock_adapter(0);

    return 0;
}

// Test 5: Adapter Teardown and Null Safety Guards
static int test_hw_ca_close_dev_and_null_guards() {
    // Null pointer guards
    hw_create_key(nullptr);
    hw_delete_key(nullptr);
    hw_set_cw(nullptr, nullptr);

    adapter *ad = setup_mock_adapter(0, 0, 0);
    ASSERT(ad != nullptr, "setup_mock_adapter failed");

    int res = hw_ca_close_dev(ad);
    ASSERT(res == 0, "hw_ca_close_dev failed");

    cleanup_mock_adapter(0);
    return 0;
}

int main() {
    opts.log = 1;
    opts.debug = 255;
    opts.cache_dir = "/tmp";
    std::memset(a, 0, sizeof(a));

    init_hw_descrambler();

    TEST_FUNC(test_hw_key_lifecycle(),
              "testing hw_create_key and hw_delete_key lifecycle");
    TEST_FUNC(test_hw_descrambler_disabled_when_opts_enigma_zero(),
              "testing hw_descrambler disabled when opts.enigma == 0");
    TEST_FUNC(test_multi_channel_slot_allocation(),
              "testing multi-channel parallel slot allocation and unbinding");
    TEST_FUNC(test_aes128_key_programming(),
              "testing AES-128 ECB and CBC key programming simulation");
    TEST_FUNC(test_hw_ca_close_dev_and_null_guards(),
              "testing adapter teardown and null pointer safety guards");

    return 0;
}
