#include "utils/logging/logging.h"

#include <stdint.h>

#define TEST_FUNC(a, str, ...)                                                 \
    {                                                                          \
        int _tmp_var;                                                          \
        if ((_tmp_var = (a))) {                                                \
            LOG(#a " failed with message: " str, ##__VA_ARGS__);               \
            return _tmp_var;                                                   \
        } else                                                                 \
            LOG("%-40s OK", #a);                                               \
    }

#define ASSERT(cond, msg)                                                      \
    if (!(cond))                                                               \
    LOG_AND_RETURN(1, "%s:%d %s: %s", __FILE__, __LINE__, __FUNCTION__, msg)

// Evaluates each side once and keeps the values for the log.
template <typename A, typename B>
static inline bool assert_equal_values(const A &a, const B &b, intmax_t *va,
                                       intmax_t *vb) {
    *va = (intmax_t)a;
    *vb = (intmax_t)b;
    return a == b;
}

#define ASSERT_EQUAL(v1, v2, msg)                                              \
    if (intmax_t _ae_v1, _ae_v2;                                               \
        !assert_equal_values((v1), (v2), &_ae_v1, &_ae_v2))                    \
    LOG_AND_RETURN(1, "%s:%d %s: Expected '%jd', got '%jd': %s", __FILE__,     \
                   __LINE__, __FUNCTION__, _ae_v1, _ae_v2, msg)

#define writev(a, b, c) _writev(a, b, c)
