// Hardware Descrambler implementation for Enigma2 DVB CA ioctls (C++23)

#include "hw_descrambler.h"
#include "adapter.h"
#include "minisatip.h"
#include "opts.h"
#include "pmt.h"
#include "tables.h"
#include "utils.h"

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <mutex>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>

namespace {
constexpr int DEFAULT_MAX_DESCRAMBLERS = 16;
constexpr int MAX_SLOTS = 64;

#define DEFAULT_LOG LOG_DVBCA

struct HwKey {
    int ca_fd{-1};
    int adapter_id{-1};
    int pmt_id{-1};
    int slot_index{-1};
    int algo{-1};
    int num_descramblers{DEFAULT_MAX_DESCRAMBLERS};

    explicit HwKey(int algorithm) : algo(algorithm) {}
};

std::string get_ca_device_path(const adapter *ad) {
    if (!ad)
        return "";
    return "/dev/dvb/adapter" + std::to_string(ad->pa) + "/ca" +
           std::to_string(ad->fn);
}

int get_max_descramblers(int ca_fd) {
    struct ca_descr_info info{};
    if (ioctl(ca_fd, CA_GET_DESCR_INFO, &info) == 0 && info.num > 0) {
        LOG("hw_descrambler: CA_GET_DESCR_INFO returned %u hardware "
            "descrambler slots",
            info.num);
        return info.num;
    }
    LOG("hw_descrambler: CA_GET_DESCR_INFO unavailable, defaulting to %d slots",
        DEFAULT_MAX_DESCRAMBLERS);
    return DEFAULT_MAX_DESCRAMBLERS;
}

class HwSlotManager {
  private:
    struct AdapterCaState {
        int ca_fd{-1};
        int num_descramblers{DEFAULT_MAX_DESCRAMBLERS};
        std::array<int, MAX_SLOTS> pmt_slots{};
        std::unordered_map<int, std::unordered_set<int>> bound_pids;

        AdapterCaState() { pmt_slots.fill(-1); }
    };

    std::array<AdapterCaState, MAX_ADAPTERS> adapters_{};
    std::mutex mutex_{};

    AdapterCaState *get_state(int physical_adapter_id) {
        if (physical_adapter_id < 0 || physical_adapter_id >= MAX_ADAPTERS)
            return nullptr;
        return &adapters_[physical_adapter_id];
    }

  public:
    static HwSlotManager &instance() noexcept {
        static HwSlotManager mgr;
        return mgr;
    }

    int get_or_open_ca_fd(int physical_adapter_id, const char *device_path) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto *ca = get_state(physical_adapter_id);
        if (!ca)
            return -1;

        if (ca->ca_fd < 0) {
            ca->ca_fd = open(device_path, O_RDWR | O_CLOEXEC);
            if (ca->ca_fd < 0 && errno == ENOENT) {
                const std::string alt_path =
                    "/dev/ci" + std::to_string(physical_adapter_id);
                ca->ca_fd = open(alt_path.c_str(), O_RDWR | O_CLOEXEC);
                if (ca->ca_fd >= 0) {
                    LOG("hw_descrambler: opened fallback %s (ca_fd = %d) for "
                        "adapter %d",
                        alt_path.c_str(), ca->ca_fd, physical_adapter_id);
                }
            }
            if (ca->ca_fd >= 0) {
                ca->num_descramblers = get_max_descramblers(ca->ca_fd);
                if (ca->num_descramblers <= 0)
                    ca->num_descramblers = DEFAULT_MAX_DESCRAMBLERS;
                LOG("hw_descrambler: successfully opened shared %s (ca_fd = "
                    "%d, max slots = %d) for physical adapter %d",
                    device_path, ca->ca_fd, ca->num_descramblers,
                    physical_adapter_id);
            } else {
                LOG("hw_descrambler: failed to open %s: %s (errno %d)",
                    device_path, std::strerror(errno), errno);
            }
        }
        return ca->ca_fd;
    }

    int get_max_slots(int physical_adapter_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto *ca = get_state(physical_adapter_id);
        if (!ca)
            return DEFAULT_MAX_DESCRAMBLERS;
        return ca->num_descramblers > 0 ? ca->num_descramblers
                                        : DEFAULT_MAX_DESCRAMBLERS;
    }

    int allocate_slot(int physical_adapter_id, int pmt_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto *ca = get_state(physical_adapter_id);
        if (!ca)
            return -1;

        if (ca->num_descramblers <= 0)
            ca->num_descramblers = DEFAULT_MAX_DESCRAMBLERS;

        const int max_desc = std::min(ca->num_descramblers, MAX_SLOTS);

        for (int i = 0; i < max_desc; ++i) {
            if (ca->pmt_slots[i] == pmt_id) {
                return i;
            }
        }

        for (int i = 0; i < max_desc; ++i) {
            if (ca->pmt_slots[i] == -1) {
                ca->pmt_slots[i] = pmt_id;
                LOG("hw_descrambler: Allocated hardware descrambler slot index "
                    "%d for PMT %d on physical adapter %d",
                    i, pmt_id, physical_adapter_id);
                return i;
            }
        }

        LOG("hw_descrambler: Error: all slots full on physical adapter %d for "
            "PMT %d",
            physical_adapter_id, pmt_id);
        return -1;
    }

    void release_slot(int physical_adapter_id, int pmt_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto *ca = get_state(physical_adapter_id);
        if (!ca)
            return;

        const int max_desc = std::min(ca->num_descramblers, MAX_SLOTS);

        for (int i = 0; i < max_desc; ++i) {
            if (ca->pmt_slots[i] == pmt_id) {
                LOG("hw_descrambler: Released hardware descrambler slot index "
                    "%d for PMT %d on physical adapter %d",
                    i, pmt_id, physical_adapter_id);
                ca->pmt_slots[i] = -1;
                break;
            }
        }
    }

    // Swap in the new bound set, returning pids to unbind.
    std::unordered_set<int>
    retarget_pids(int physical_adapter_id, int pmt_id,
                  const std::unordered_set<int> &current) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto *ca = get_state(physical_adapter_id);
        if (!ca)
            return {};
        std::unordered_set<int> removed;
        for (int pid : ca->bound_pids[pmt_id])
            if (!current.count(pid))
                removed.insert(pid);
        ca->bound_pids[pmt_id] = current;
        return removed;
    }

    // Take and forget the bound set for teardown.
    std::unordered_set<int> take_bound_pids(int physical_adapter_id,
                                            int pmt_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto *ca = get_state(physical_adapter_id);
        if (!ca)
            return {};
        auto it = ca->bound_pids.find(pmt_id);
        if (it == ca->bound_pids.end())
            return {};
        std::unordered_set<int> bound = it->second;
        ca->bound_pids.erase(it);
        return bound;
    }

    std::unordered_set<int> bound_pids(int physical_adapter_id, int pmt_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto *ca = get_state(physical_adapter_id);
        if (!ca)
            return {};
        auto it = ca->bound_pids.find(pmt_id);
        if (it == ca->bound_pids.end())
            return {};
        return it->second;
    }

    void close_adapter_ca(int physical_adapter_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto *ca = get_state(physical_adapter_id);
        if (!ca)
            return;

        if (ca->ca_fd >= 0) {
            LOG("hw_descrambler: closing shared ca_fd %d for physical adapter "
                "%d",
                ca->ca_fd, physical_adapter_id);
            close(ca->ca_fd);
            ca->ca_fd = -1;
        }
        ca->pmt_slots.fill(-1);
        ca->bound_pids.clear();
    }
};
} // namespace

void hw_create_key(SCW *cw) {
    if (!cw)
        return;
    auto k = std::make_unique<HwKey>(cw->algo);
    LOG("hw_descrambler: hw_create_key cw_id = %d, algo = %d", cw->id,
        cw->algo);
    cw->key = k.release(); // Implicit conversion from HwKey* to void*
}

void hw_delete_key(SCW *cw) {
    if (!cw || !cw->key)
        return;
    auto *k = static_cast<HwKey *>(cw->key);
    delete k;
    cw->key = nullptr;
}

void hw_set_cw(SCW *cw, SPMT *pmt) {
    if (!opts.hw_descrambler) {
        LOG("hw_descrambler: --hw-descrambler not enabled, skipping hw_set_cw");
        return;
    }

    if (!cw || !cw->key || !pmt) {
        LOG("hw_descrambler: hw_set_cw key or pmt is nullptr");
        return;
    }

    if (cw->algo == CA_ALGO_DVBCSA_ICAM) {
        LOG("hw_descrambler: ICAM mode (CA_ALGO_DVBCSA_ICAM) not supported by "
            "hardware descrambler");
        return;
    }

    auto *k = static_cast<HwKey *>(cw->key);
    adapter *ad = get_adapter(pmt->adapter);
    if (!ad) {
        LOG("hw_descrambler: adapter for PMT %d not found", pmt->adapter);
        return;
    }

    const std::string device_path = get_ca_device_path(ad);

    int ca_fd = HwSlotManager::instance().get_or_open_ca_fd(
        ad->pa, device_path.c_str());
    if (ca_fd < 0) {
        LOG("hw_descrambler: unable to obtain ca_fd for %s",
            device_path.c_str());
        return;
    }

    k->ca_fd = ca_fd;
    k->adapter_id = ad->pa;
    const int pmt_id = pmt->id;
    k->pmt_id = pmt_id;
    k->num_descramblers = HwSlotManager::instance().get_max_slots(ad->pa);

    // Dynamically allocate or retrieve hardware slot index for this PMT
    k->slot_index = HwSlotManager::instance().allocate_slot(ad->pa, pmt_id);
    if (k->slot_index < 0) {
        LOG("hw_descrambler: slot allocation failed for PMT %d on adapter %d",
            pmt_id, ad->pa);
        return;
    }

    LOG("hw_descrambler: hw_set_cw called for PMT %d, adapter %d "
        "(pa %d, ca %d), slot %d/%d, algo %d, parity %d",
        pmt->id, pmt->adapter, ad->pa, ad->fn, k->slot_index,
        k->num_descramblers, cw->algo, cw->parity);

    // 1. Configure Hardware Descrambler Algorithm Mode using pmt.h CA_ALGO_*
    // values
    struct ca_descr_mode mode_cmd{};
    mode_cmd.index = k->slot_index;
    mode_cmd.algo = cw->algo;
    mode_cmd.cipher_mode = (cw->algo == CA_ALGO_AES128_CBC) ? 1 : 0;

    const int res_mode = ioctl(k->ca_fd, CA_SET_DESCR_MODE, &mode_cmd);
    LOG("hw_descrambler: CA_SET_DESCR_MODE ioctl (slot=%d, algo=%d, mode=%d) "
        "-> result=%d%s",
        mode_cmd.index, mode_cmd.algo, mode_cmd.cipher_mode, res_mode,
        res_mode < 0 ? std::strerror(errno) : " SUCCESS");

    // 2. Bind PMT PIDs to Hardware Descrambler Slot
    for (std::size_t i = 0; i < pmt->stream_pids.size(); ++i) {
        struct ca_pid pid_cmd{};
        pid_cmd.pid = pmt->stream_pids[i].pid;
        pid_cmd.index = k->slot_index;
        const int res_pid = ioctl(k->ca_fd, CA_SET_PID, &pid_cmd);
        LOG("hw_descrambler: CA_SET_PID ioctl (pid=%d, slot=%d) -> result=%d%s",
            pid_cmd.pid, pid_cmd.index, res_pid,
            res_pid < 0 ? std::strerror(errno) : " SUCCESS");
    }

    // 3. Program Control Word (CW) and IV into Hardware Registers
    if (cw->algo == CA_ALGO_DVBCSA) {
        struct ca_descr descr{};
        descr.index = k->slot_index;
        descr.parity = cw->parity;
        std::memcpy(descr.cw, cw->cw, 8);
        const int res_descr = ioctl(k->ca_fd, CA_SET_DESCR, &descr);
        LOG("hw_descrambler: CA_SET_DESCR (DVBCSA) ioctl (slot=%d, parity=%d) "
            "-> result=%d%s",
            descr.index, descr.parity, res_descr,
            res_descr < 0 ? std::strerror(errno) : " SUCCESS");
    } else {
        // AES-128 16-byte Key
        struct ca_descr_data data_cmd{};
        data_cmd.index = k->slot_index;
        data_cmd.parity = cw->parity;
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
            data_cmd = {};
            data_cmd.index = k->slot_index;
            data_cmd.parity = cw->parity;
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

// Drop pids another PMT on the same hardware still lists: the
// unbind is device-wide and must not break other services.
static void hw_drop_held_pids(adapter *ad, SPMT *pmt,
                              std::unordered_set<int> &pids) {
    for (auto it = pids.begin(); it != pids.end();) {
        int held = 0;
        for (int i = 0; i < MAX_PMT && !held; i++) {
            SPMT *o = get_pmt(i);
            adapter *oa;
            if (!o || o == pmt || o->adapter < 0)
                continue;
            oa = get_adapter_nw(o->adapter);
            if (!oa || oa->pa != ad->pa)
                continue;
            for (const auto &sp : o->stream_pids)
                if (sp.pid == *it)
                    held = 1;
        }
        if (held)
            it = pids.erase(it);
        else
            ++it;
    }
}

static void hw_unbind_pids(int pa, const char *device_path,
                           const std::unordered_set<int> &pids) {
    if (pids.empty())
        return;
    int ca_fd = HwSlotManager::instance().get_or_open_ca_fd(pa, device_path);
    if (ca_fd < 0)
        return;
    for (int pid : pids) {
        struct ca_pid pid_cmd{};
        pid_cmd.pid = pid;
        pid_cmd.index = -1; // -1 unbinds PID
        ioctl(ca_fd, CA_SET_PID, &pid_cmd);
    }
}

int hw_ca_del_pmt(adapter *ad, SPMT *pmt) {
    if (!opts.hw_descrambler || !ad || !pmt)
        return 0;

    const int pmt_id = pmt->id;
    LOG("hw_descrambler: hw_ca_del_pmt called for PMT %d on "
        "physical adapter %d (logical %d)",
        pmt->id, ad->pa, ad->id);

    // Unbind everything ever bound: updates shrink the live list, so
    // unbinding just it would leak removed pids.
    std::unordered_set<int> bound =
        HwSlotManager::instance().take_bound_pids(ad->pa, pmt_id);
    for (const auto &sp : pmt->stream_pids)
        bound.insert(sp.pid);
    hw_drop_held_pids(ad, pmt, bound);
    const std::string device_path = get_ca_device_path(ad);
    hw_unbind_pids(ad->pa, device_path.c_str(), bound);

    // 2. Release hardware descrambler slot index
    HwSlotManager::instance().release_slot(ad->pa, pmt_id);
    return 0;
}

std::unordered_set<int> hw_bound_pids_for_test(int pa, int pmt_id) {
    return HwSlotManager::instance().bound_pids(pa, pmt_id);
}

// On re-send, unbind pids the update dropped; added pids bind on
// the next CW via hw_set_cw, which binds the whole live list.
int hw_ca_add_pmt(adapter *ad, SPMT *pmt, int update) {
    if (!opts.hw_descrambler || !ad || !pmt)
        return TABLES_RESULT_OK;
    std::unordered_set<int> current;
    for (const auto &sp : pmt->stream_pids)
        current.insert(sp.pid);
    std::unordered_set<int> removed =
        HwSlotManager::instance().retarget_pids(ad->pa, pmt->id, current);
    LOG("hw_descrambler: PMT %d %s: %zu pids bound, %zu removed", pmt->id,
        update ? "update" : "add", current.size(), removed.size());
    hw_drop_held_pids(ad, pmt, removed);
    const std::string device_path = get_ca_device_path(ad);
    hw_unbind_pids(ad->pa, device_path.c_str(), removed);
    return TABLES_RESULT_OK;
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

int hw_ca_init_dev(adapter *ad) {
    (void)ad;
    return TABLES_RESULT_OK;
}

static SCA_op hw_ca_op{};

void init_hw_descrambler() {
    if (opts.hw_descrambler) {
        LOG("hw_descrambler: Initializing Hardware Descrambler "
            "(--hw-descrambler enabled)");
        register_algo(&hw_csa_op);
        register_algo(&hw_aes_ecb_op);
        register_algo(&hw_aes_cbc_op);

        hw_ca_op = {};
        hw_ca_op.ca_add_pmt = hw_ca_add_pmt;
        hw_ca_op.ca_init_dev =
            reinterpret_cast<ca_device_action>(hw_ca_init_dev);
        hw_ca_op.ca_del_pmt = reinterpret_cast<ca_pmt_action>(hw_ca_del_pmt);
        hw_ca_op.ca_close_dev =
            reinterpret_cast<ca_device_action>(hw_ca_close_dev);
        add_ca(&hw_ca_op);
    } else {
        LOG("hw_descrambler: --hw-descrambler not enabled, hardware "
            "descrambler disabled");
    }
}
