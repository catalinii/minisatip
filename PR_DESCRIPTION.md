# Feature: Enigma2 Hardware Descrambler Integration (DVB-CSA & AES-128)

## Summary
This pull request implements native **Hardware Descrambler** support for Enigma2 STBs using Linux DVB CA ioctls (`/dev/dvb/adapterX/caY`). It enables direct hardware descrambling for DVB-CSA, AES-128 ECB, and AES-128 CBC streams on Enigma2 receivers, bypassing software decryption loops and dramatically improving CPU efficiency.

---

## Key Features & Architecture

### 1. Enigma2 DVB CA Extended Ioctls (`src/hw_descrambler.h` & `src/hw_descrambler.cpp`)
- **`CA_GET_DESCR_INFO`**: Dynamically queries the receiver hardware to detect total available hardware descrambler slots (e.g. 16 slots).
- **`CA_SET_DESCR_MODE`**: Configures the hardware slot algorithm (`CaAlgo::DvbCsa`, `CaAlgo::Aes128Ecb`, `CaAlgo::Aes128Cbc`) and cipher mode (`CipherMode::Ecb`, `CipherMode::Cbc`).
- **`CA_SET_PID`**: Binds stream PIDs (video, audio, subtitles) to specific hardware descrambler slot indices. Passing `index = -1` unbinds PIDs upon channel stop.
- **`CA_SET_DESCR`**: Programs 8-byte DVB-CSA EVEN (parity 0) and ODD (parity 1) Control Words directly into hardware descrambler registers.
- **`CA_SET_DESCR_DATA`**: Programs 16-byte AES-128 Keys (`data_type = 0`) and 16-byte IVs (`data_type = 1`) into hardware registers for AES-encrypted streams (e.g. 4K HEVC channels).

### 2. Thread-Safe Dynamic Slot Allocator (`HwSlotManager`)
- Implemented a thread-safe singleton (`HwSlotManager`) to allocate and release hardware descrambler slot indices per physical DVB adapter (`ad->pa`).
- **Parity Alignment**: EVEN (parity 0) and ODD (parity 1) Control Words for the same PMT/service share the exact same hardware slot index (`slot_index = allocate_slot(ad->pa, pmt_id)`).
- **Shared `ca_fd` Management**: Maintained a single, shared open CA file descriptor (`ca_fd`) per physical adapter using `O_CLOEXEC`. This resolves Linux DVB driver `O_EXCL` exclusive-open restrictions and eliminates `EBUSY (Device or resource busy, errno 16)` errors during concurrent multi-channel streaming.

### 3. Automatic Lifecycle Management (`hw_ca_del_pmt`)
- Registered `hw_ca_del_pmt` with the `SCA_op` CA interface (`ca_del_pmt`).
- When a channel is stopped or closed (`pmt_del` / `close_pmt_for_cas`), stream PIDs are automatically unbound (`CA_SET_PID` with `index = -1`) and the hardware slot index is returned to the free pool for dynamic reuse.
- `stop_cw` is set to `nullptr` to align with DVB-CSA/AES software operations, preventing playback glitches during key rotation.

---

## Unit Testing (`tests/test_hw_descrambler.cpp`)
Added a new unit test suite to CMake (`ctest` test target `#13`):
- `test_hw_key_lifecycle`: Validates key creation/deletion for CSA and AES algorithms.
- `test_hw_descrambler_disabled_when_opts_enigma_zero`: Validates fallback when `opts.enigma == 0`.
- `test_multi_channel_slot_allocation`: Validates multi-channel slot allocation, parity alignment, and slot unbinding/reclaiming.
- `test_aes128_key_programming`: Simulates AES-128 ECB and CBC key/IV programming.
- `test_hw_ca_close_dev_and_null_guards`: Validates adapter teardown and null pointer guards.

---

## Live Hardware Validation
Verified on Enigma2 ARM STB receiver hardware:

1. **Multi-Channel Concurrent Descrambling (314 MHz DVB-C)**:
   - Streamed multiple HD encrypted channels simultaneously.
   - Performed rapid channel switching across 4 channels while maintaining a continuous background stream.
   - Result: **0 frame drops, 0 corrupt packets, 0 `EBUSY` errors**.

2. **4K HEVC AES-128 Descrambling (498 MHz DVB-C)**:
   - Streamed multiple 4K HEVC 10-bit HDR @ 50fps AES-encrypted channels.
   - Result: **500 frames decoded cleanly at 3.99x speed with 0 errors**.

---

## Changed Files
- `src/hw_descrambler.h`: Added Enigma2 extended CA ioctl structs and function prototypes.
- `src/hw_descrambler.cpp`: Core hardware descrambler implementation, `HwSlotManager`, and CA hooks.
- `src/pmt.cpp`: Initialized `init_hw_descrambler()` in algorithm registry.
- `CMakeLists.txt`: Added `src/hw_descrambler.cpp` and `tests/test_hw_descrambler.cpp` test target.
- `tests/test_hw_descrambler.cpp`: New automated unit test suite.
