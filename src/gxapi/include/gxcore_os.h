#ifndef GXCORE_OS
#define GXCORE_OS

#define GXCORE_RETURN_START             (0)
#define GXCORE_RETURN_TRUE(n)           (n + GXCORE_RETURN_START)
#define GXCORE_RETURN_ERROR(n)          (-(n + GXCORE_RETURN_START))


#define GXCORE_MOD_UNEXIST              (GXCORE_RETURN_TRUE(2))
#define GXCORE_MOD_EXIST                (GXCORE_RETURN_TRUE(1))
#define GXCORE_FILE_UNEXIST             (GXCORE_RETURN_TRUE(2))
#define GXCORE_FILE_EXIST               (GXCORE_RETURN_TRUE(1))
#define GXCORE_SUCCESS                  (0)
#define GXCORE_ERROR                    (GXCORE_RETURN_ERROR(1))
#define GXCORE_INVALID_POINTER          (GXCORE_RETURN_ERROR(2))
#define GXCORE_INVALID_ID               (GXCORE_RETURN_ERROR(3))
#define GXCORE_SEM_FAILURE              (GXCORE_RETURN_ERROR(4))
#define GXCORE_SEM_TIMEOUT              (GXCORE_RETURN_ERROR(5))
#define GXCORE_QUEUE_EMPTY              (GXCORE_RETURN_ERROR(6))
#define GXCORE_QUEUE_FULL               (GXCORE_RETURN_ERROR(7))
#define GXCORE_QUEUE_TIMEOUT            (GXCORE_RETURN_ERROR(8))
#define GXCORE_QUEUE_INVALID_SIZE       (GXCORE_RETURN_ERROR(9))
#define GXCORE_ERR_NO_FREE_IDS          (GXCORE_RETURN_ERROR(10))
#define GXCORE_INVALID_PRIORITY         (GXCORE_RETURN_ERROR(11))
#define GXCORE_LOCK_FAILURE             (GXCORE_RETURN_ERROR(12))
#define GXCORE_THREAD_BE_EXITED         (GXCORE_RETURN_ERROR(13))
#define GXCORE_OVERFLOW                 (GXCORE_RETURN_ERROR(14))
#define GXCORE_COND_FAILURE             (GXCORE_RETURN_ERROR(15))

/* Defines for Queue Timeout parameters */
#define GXCORE_PEND   (0)
#define GXCORE_CHECK (-1)

#include <gxtype.h>
/* Include the OS API modules */

#endif
