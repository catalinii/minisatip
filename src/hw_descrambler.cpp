/*
 * Hardware Descrambler implementation for Enigma2 DVB CA ioctls (C++23)
 */

#include "hw_descrambler.h"
#include "adapter.h"
#include "minisatip.h"
#include "opts.h"
#include "pmt.h"
#include "tables.h"
#include "utils.h"

#include <array>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <mutex>
#include <string_view>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {
constexpr int kDefaultLog = LOG_DVBCA;
constexpr int kDefaultMaxDescramblers = 16;
constexpr std::size_t kMaxSlots = 64;

#define DEFAULT_LOG kDefaultLog

enum class CaAlgo : std::uint32_t {
    DvbCsa = 0,
    Des = 1,
    Aes128Ecb = 2,
    Aes128Cbc = 3
};

enum class CipherMode : std::uint32_t { Ecb = 0, Cbc = 1 };

struct HwKey {
    int ca_fd{-1};
    int adapter_id{-1};
    int pmt_id{-1};
    int slot_index{-1};
    int algo{-1};
    int num_descramblers{kDefaultMaxDescramblers};

    explicit HwKey(int algorithm) : algo(algorithm) {}
};

int get_max_descramblers(int ca_fd) {
    struct ca_descr_info info{};
    if (ioctl(ca_fd, CA_GET_DESCR_INFO, &info) == 0 && info.num > 0) {
        LOG("hw_descrambler: CA_GET_DESCR_INFO returned %u hardware "
            "descrambler slots",
            info.num);
        return static_cast<int>(info.num);
    }
    LOG("hw_descrambler: CA_GET_DESCR_INFO unavailable, defaulting to %d slots",
        kDefaultMaxDescramblers);
    return kDefaultMaxDescramblers;
}

class HwSlotManager {
  private:
    struct AdapterCaState {
        int ca_fd{-1};
        int num_descramblers{kDefaultMaxDescramblers};
        std::array<int, kMaxSlots> pmt_slots{};

        AdapterCaState() { pmt_slots.fill(-1); }
    };

    std::array<AdapterCaState, MAX_ADAPTERS> adapters_{};
    std::mutex mutex_{};

  public:
    static HwSlotManager &instance() noexcept {
        static HwSlotManager mgr;
        return mgr;
    }

    int get_or_open_ca_fd(int physical_adapter_id, const char *device_path) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (physical_adapter_id < 0 || physical_adapter_id >= MAX_ADAPTERS)
            return -1;

        auto &ca = adapters_[static_cast<std::size_t>(physical_adapter_id)];
        if (ca.ca_fd < 0) {
            ca.ca_fd = open(device_path, O_RDWR | O_CLOEXEC);
            if (ca.ca_fd >= 0) {
                ca.num_descramblers = get_max_descramblers(ca.ca_fd);
                if (ca.num_descramblers <= 0)
                    ca.num_descramblers = kDefaultMaxDescramblers;
                LOG("hw_descrambler: successfully opened shared %s (ca_fd = "
                    "%d, max slots = %d) for physical adapter %d",
                    device_path, ca.ca_fd, ca.num_descramblers,
                    physical_adapter_id);
            } else {
                LOG("hw_descrambler: failed to open %s: %s (errno %d)",
                    device_path, std::strerror(errno), errno);
            }
        }
        return ca.ca_fd;
    }

    int get_max_slots(int physical_adapter_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (physical_adapter_id < 0 || physical_adapter_id >= MAX_ADAPTERS)
            return kDefaultMaxDescramblers;
        int slots = adapters_[static_cast<std::size_t>(physical_adapter_id)]
                        .num_descramblers;
        return slots > 0 ? slots : kDefaultMaxDescramblers;
    }

    int allocate_slot(int physical_adapter_id, int pmt_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (physical_adapter_id < 0 || physical_adapter_id >= MAX_ADAPTERS)
            return 0;

        auto &ca = adapters_[static_cast<std::size_t>(physical_adapter_id)];
        if (ca.num_descramblers <= 0)
            ca.num_descramblers = kDefaultMaxDescramblers;

        for (int i = 0;
             i < ca.num_descramblers && i < static_cast<int>(kMaxSlots); ++i) {
            if (ca.pmt_slots[static_cast<std::size_t>(i)] == pmt_id) {
                return i;
            }
        }

        for (int i = 0;
             i < ca.num_descramblers && i < static_cast<int>(kMaxSlots); ++i) {
            if (ca.pmt_slots[static_cast<std::size_t>(i)] == -1) {
                ca.pmt_slots[static_cast<std::size_t>(i)] = pmt_id;
                LOG("hw_descrambler: Allocated hardware descrambler slot index "
                    "%d for PMT %d on physical adapter %d",
                    i, pmt_id, physical_adapter_id);
                return i;
            }
        }

        int fallback_slot = pmt_id % ca.num_descramblers;
        LOG("hw_descrambler: Warning: all slots full on physical adapter %d, "
            "using "
            "fallback slot %d for PMT %d",
            physical_adapter_id, fallback_slot, pmt_id);
        return fallback_slot;
    }

    void release_slot(int physical_adapter_id, int pmt_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (physical_adapter_id < 0 || physical_adapter_id >= MAX_ADAPTERS)
            return;

        auto &ca = adapters_[static_cast<std::size_t>(physical_adapter_id)];
        for (int i = 0;
             i < ca.num_descramblers && i < static_cast<int>(kMaxSlots); ++i) {
            if (ca.pmt_slots[static_cast<std::size_t>(i)] == pmt_id) {
                LOG("hw_descrambler: Released hardware descrambler slot index "
                    "%d for PMT %d on physical adapter %d",
                    i, pmt_id, physical_adapter_id);
                ca.pmt_slots[static_cast<std::size_t>(i)] = -1;
                break;
            }
        }
    }

    void close_adapter_ca(int physical_adapter_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (physical_adapter_id < 0 || physical_adapter_id >= MAX_ADAPTERS)
            return;

        auto &ca = adapters_[static_cast<std::size_t>(physical_adapter_id)];
        if (ca.ca_fd >= 0) {
            LOG("hw_descrambler: closing shared ca_fd %d for physical adapter "
                "%d",
                ca.ca_fd, physical_adapter_id);
            close(ca.ca_fd);
            ca.ca_fd = -1;
        }
        ca.pmt_slots.fill(-1);
    }
};
} // namespace

void hw_create_key(SCW *cw) {
    if (!cw)
        return;
    auto k = std::make_unique<HwKey>(cw->algo);
    LOG("hw_descrambler: hw_create_key cw_id = %d, algo = %d", cw->id,
        cw->algo);
    cw->key = static_cast<void *>(k.release());
}

void hw_delete_key(SCW *cw) {
    if (!cw || !cw->key)
        return;
    auto *k = static_cast<HwKey *>(cw->key);
    delete k;
    cw->key = nullptr;
}

void hw_set_cw(SCW *cw, SPMT *pmt) {
    if (!opts.enigma) {
        LOG("hw_descrambler: opts.enigma is 0, skipping hw_set_cw");
        return;
    }

    if (!cw || !cw->key || !pmt) {
        LOG("hw_descrambler: hw_set_cw key or pmt is nullptr");
        return;
    }

    auto *k = static_cast<HwKey *>(cw->key);
    adapter *ad = get_adapter(pmt->adapter);
    if (!ad) {
        LOG("hw_descrambler: adapter for PMT %d not found", pmt->adapter);
        return;
    }

    std::array<char, 64> device_path{};
    std::snprintf(device_path.data(), device_path.size(),
                  "/dev/dvb/adapter%d/ca%d", ad->pa, ad->fn);

    int ca_fd =
        HwSlotManager::instance().get_or_open_ca_fd(ad->pa, device_path.data());
    if (ca_fd < 0) {
        LOG("hw_descrambler: unable to obtain ca_fd for %s",
            device_path.data());
        return;
    }

    k->ca_fd = ca_fd;
    k->adapter_id = ad->pa;
    const int pmt_id = (pmt->master_pmt >= 0) ? pmt->master_pmt : pmt->id;
    k->pmt_id = pmt_id;
    k->num_descramblers = HwSlotManager::instance().get_max_slots(ad->pa);

    // Dynamically allocate or retrieve hardware slot index for this PMT
    k->slot_index = HwSlotManager::instance().allocate_slot(ad->pa, pmt_id);

    LOG("hw_descrambler: hw_set_cw called for PMT %d (master %d), adapter %d "
        "(pa %d, ca %d), slot %d/%d, algo %d, parity %d",
        pmt->id, pmt_id, pmt->adapter, ad->pa, ad->fn, k->slot_index,
        k->num_descramblers, cw->algo, cw->parity);

    // 1. Configure Hardware Descrambler Algorithm Mode
    struct ca_descr_mode mode_cmd{};
    mode_cmd.index = static_cast<unsigned int>(k->slot_index);

    if (cw->algo == CA_ALGO_DVBCSA) {
        mode_cmd.algo = static_cast<unsigned int>(CaAlgo::DvbCsa);
        mode_cmd.cipher_mode = static_cast<unsigned int>(CipherMode::Ecb);
    } else if (cw->algo == CA_ALGO_AES128_ECB) {
        mode_cmd.algo = static_cast<unsigned int>(CaAlgo::Aes128Ecb);
        mode_cmd.cipher_mode = static_cast<unsigned int>(CipherMode::Ecb);
    } else if (cw->algo == CA_ALGO_AES128_CBC) {
        mode_cmd.algo = static_cast<unsigned int>(CaAlgo::Aes128Cbc);
        mode_cmd.cipher_mode = static_cast<unsigned int>(CipherMode::Cbc);
    }

    const int res_mode = ioctl(k->ca_fd, CA_SET_DESCR_MODE, &mode_cmd);
    LOG("hw_descrambler: CA_SET_DESCR_MODE ioctl (slot=%d, algo=%d, mode=%d) "
        "-> result=%d%s",
        mode_cmd.index, mode_cmd.algo, mode_cmd.cipher_mode, res_mode,
        res_mode < 0 ? std::strerror(errno) : " SUCCESS");

    // 2. Bind PMT PIDs to Hardware Descrambler Slot
    for (std::size_t i = 0; i < pmt->stream_pids.size(); ++i) {
        struct ca_pid pid_cmd{};
        pid_cmd.pid = static_cast<unsigned int>(pmt->stream_pids[i].pid);
        pid_cmd.index = k->slot_index;
        const int res_pid = ioctl(k->ca_fd, CA_SET_PID, &pid_cmd);
        LOG("hw_descrambler: CA_SET_PID ioctl (pid=%d, slot=%d) -> result=%d%s",
            pid_cmd.pid, pid_cmd.index, res_pid,
            res_pid < 0 ? std::strerror(errno) : " SUCCESS");
    }

    // 3. Program Control Word (CW) and IV into Hardware Registers
    if (cw->algo == CA_ALGO_DVBCSA) {
        struct ca_descr descr{};
        descr.index = static_cast<unsigned int>(k->slot_index);
        descr.parity = static_cast<unsigned int>(cw->parity);
        std::memcpy(descr.cw, cw->cw, 8);
        const int res_descr = ioctl(k->ca_fd, CA_SET_DESCR, &descr);
        LOG("hw_descrambler: CA_SET_DESCR (DVBCSA) ioctl (slot=%d, parity=%d, "
            "CW=%02X%02X%02X%02X...) -> result=%d%s",
            descr.index, descr.parity, descr.cw[0], descr.cw[1], descr.cw[2],
            descr.cw[3], res_descr,
            res_descr < 0 ? std::strerror(errno) : " SUCCESS");
    } else {
        // AES-128 16-byte Key
        struct ca_descr_data data_cmd{};
        data_cmd.index = static_cast<unsigned int>(k->slot_index);
        data_cmd.parity = static_cast<unsigned int>(cw->parity);
        data_cmd.data_type = 0; // 0 = CW / Key
        data_cmd.data_len = 16;
        std::memcpy(data_cmd.data, cw->cw, 16);
        const int res_key = ioctl(k->ca_fd, CA_SET_DESCR_DATA, &data_cmd);
        LOG("hw_descrambler: CA_SET_DESCR_DATA (AES Key) ioctl (slot=%d, "
            "parity=%d) -> result=%d%s",
            data_cmd.index, data_cmd.parity, res_key,
            res_key < 0 ? std::strerror(errno) : " SUCCESS");

        // AES-128 16-byte IV (for CBC mode)
        if (cw->algo == CA_ALGO_AES128_CBC) {
            std::memset(&data_cmd, 0, sizeof(data_cmd));
            data_cmd.index = static_cast<unsigned int>(k->slot_index);
            data_cmd.parity = static_cast<unsigned int>(cw->parity);
            data_cmd.data_type = 1; // 1 = IV
            data_cmd.data_len = 16;
            std::memcpy(data_cmd.data, cw->iv, 16);
            const int res_iv = ioctl(k->ca_fd, CA_SET_DESCR_DATA, &data_cmd);
            LOG("hw_descrambler: CA_SET_DESCR_DATA (AES IV) ioctl (slot=%d, "
                "parity=%d) -> result=%d%s",
                data_cmd.index, data_cmd.parity, res_iv,
                res_iv < 0 ? std::strerror(errno) : " SUCCESS");
        }
    }
}

void hw_decrypt_stream(SCW *cw, SPMT_batch *batch, int batch_len) {
    (void)cw;
    (void)batch;
    (void)batch_len;
    // Hardware descrambler decrypts TS stream directly in STB demux/CA
    // hardware. Software decryption loop is bypassed (pass-through).
}

int hw_ca_del_pmt(adapter *ad, SPMT *pmt) {
    if (!opts.enigma || !ad || !pmt)
        return 0;

    const int pmt_id = (pmt->master_pmt >= 0) ? pmt->master_pmt : pmt->id;
    LOG("hw_descrambler: hw_ca_del_pmt called for PMT %d (master %d) on "
        "physical adapter %d (logical %d)",
        pmt->id, pmt_id, ad->pa, ad->id);

    std::array<char, 64> device_path{};
    std::snprintf(device_path.data(), device_path.size(),
                  "/dev/dvb/adapter%d/ca%d", ad->pa, ad->fn);
    int ca_fd =
        HwSlotManager::instance().get_or_open_ca_fd(ad->pa, device_path.data());
    if (ca_fd >= 0) {
        // 1. Unbind stream PIDs from hardware descrambler slot
        for (std::size_t i = 0; i < pmt->stream_pids.size(); ++i) {
            struct ca_pid pid_cmd{};
            pid_cmd.pid = static_cast<unsigned int>(pmt->stream_pids[i].pid);
            pid_cmd.index = -1; // -1 unbinds PID
            ioctl(ca_fd, CA_SET_PID, &pid_cmd);
        }
    }

    // 2. Release hardware descrambler slot index
    HwSlotManager::instance().release_slot(ad->pa, pmt_id);
    return 0;
}

int hw_ca_close_dev(adapter *ad) {
    if (ad) {
        HwSlotManager::instance().close_adapter_ca(ad->pa);
    }
    return 0;
}

SCW_op hw_csa_op = {.algo = CA_ALGO_DVBCSA,
                    .create_cw = reinterpret_cast<Create_CW>(hw_create_key),
                    .delete_cw = reinterpret_cast<Delete_CW>(hw_delete_key),
                    .set_cw = reinterpret_cast<Set_CW>(hw_set_cw),
                    .stop_cw = nullptr,
                    .decrypt_stream =
                        reinterpret_cast<Decrypt_Stream>(hw_decrypt_stream)};

SCW_op hw_aes_ecb_op = {
    .algo = CA_ALGO_AES128_ECB,
    .create_cw = reinterpret_cast<Create_CW>(hw_create_key),
    .delete_cw = reinterpret_cast<Delete_CW>(hw_delete_key),
    .set_cw = reinterpret_cast<Set_CW>(hw_set_cw),
    .stop_cw = nullptr,
    .decrypt_stream = reinterpret_cast<Decrypt_Stream>(hw_decrypt_stream)};

SCW_op hw_aes_cbc_op = {
    .algo = CA_ALGO_AES128_CBC,
    .create_cw = reinterpret_cast<Create_CW>(hw_create_key),
    .delete_cw = reinterpret_cast<Delete_CW>(hw_delete_key),
    .set_cw = reinterpret_cast<Set_CW>(hw_set_cw),
    .stop_cw = nullptr,
    .decrypt_stream = reinterpret_cast<Decrypt_Stream>(hw_decrypt_stream)};

static SCA_op hw_ca_op{};

void init_hw_descrambler() {
    if (opts.enigma) {
        LOG("hw_descrambler: Initializing Hardware Descrambler for Enigma2 "
            "(opts.enigma = %d)",
            opts.enigma);
        register_algo(&hw_csa_op);
        register_algo(&hw_aes_ecb_op);
        register_algo(&hw_aes_cbc_op);

        std::memset(&hw_ca_op, 0, sizeof(hw_ca_op));
        hw_ca_op.ca_del_pmt = reinterpret_cast<ca_pmt_action>(hw_ca_del_pmt);
        hw_ca_op.ca_close_dev =
            reinterpret_cast<ca_device_action>(hw_ca_close_dev);
        add_ca(&hw_ca_op);
    } else {
        LOG("hw_descrambler: opts.enigma is 0, hardware descrambler disabled");
    }
}
