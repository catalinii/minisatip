#include "utils/logging/logging.h"

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

#define writev(a, b, c) _writev(a, b, c)
