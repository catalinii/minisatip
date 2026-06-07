# Claude Code - minisatip Development Notes

## Fixed Issues

### TCP Transport Duplicate Socket Registration (March 2026)

**Problem:**
For TCP transport SAT>IP adapters, the same socket handle was being registered twice in the sockets list:
1. First in `satipc_open_rtsp_socket()` via `adapter_set_dvr(ad)` at line 354
2. Second in `init_hw()` via `adapter_set_dvr(ad)` at line 309 of adapter.cpp

This created duplicate socket entries that confused the socket lifecycle management:
- TCP socket handle (e.g., handle 8) was added as sock 9 and sock 10
- Cleanup logic would only properly remove the first entry, leaving the second dangling
- Later errors ("Bad file descriptor") would occur when trying to use the orphaned socket

**Root Cause:**
`satipc_open_device()` is called from `init_hw()` to initialize the adapter's open callback. During this initialization:
1. `satipc_open_device()` calls `satipc_open_rtsp_socket()`
2. `satipc_open_rtsp_socket()` calls `adapter_set_dvr()` to register the DVR socket
3. After `satipc_open_device()` returns, `init_hw()` calls `adapter_set_dvr()` AGAIN
4. This registers the same socket handle twice

**Solution:**
Added a `bool is_init` parameter to `satipc_open_rtsp_socket()`:
- When `is_init=true` (called from `satipc_open_device` during init), skip calling `adapter_set_dvr()` since `init_hw()` will call it
- When `is_init=false` (called for reconnections/error recovery), call `adapter_set_dvr()` as usual

**Files Modified:**
- `src/satipc.h`: Added `bool is_init = false` parameter to function declaration
- `src/satipc.cpp`:
  - Updated function signature with `bool is_init`
  - Added condition: `if (ad->dvr >= 0 && !is_init)` before calling `adapter_set_dvr()`
  - Updated all 4 call sites:
    - Line 679: `satipc_open_device()` initial call → `is_init=true`
    - Line 694: `satipc_open_device()` error recovery → `is_init=false`
    - Line 402: `satipc_timeout()` reconnection → `is_init=false`
    - Line 1691: `satipc_tune()` restart → `is_init=false`

**Code Style:**
- Use `bool` type for flag parameters instead of `int`
- Use `true`/`false` instead of `1`/`0`
- Document intent with comments explaining when `is_init` is true vs false

**Testing:**
- Verified TCP transport adapters no longer create duplicate socket registrations
- Clean closure without "Bad file descriptor" errors
- All adapters (TCP, UDP, SRT) initialize correctly

## Git Workflow

- **Never push directly to master**: Always create a feature branch, push to origin, request reviews on a Pull Request, and merge via PR.

