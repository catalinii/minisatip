/*	Public domain	*/

#ifndef _GX_ERROR_H_
#define _GX_ERROR_H_

/* Standard error code */
#define GX_SUCCESS     -0
#define GX_EUNDEFINED  -1		/* Undefined error */
#define GX_EPERM       -2		/* Operation not permitted */
#define GX_ENOENT      -3		/* No such file or directory */
#define GX_EINTR       -4		/* Interrupted system call */
#define GX_EIO         -5		/* Input/output error */
#define GX_E2BIG       -6		/* Argument list too long */
#define GX_EACCESS     -7		/* Permission denied */
#define GX_EBUSY       -8		/* Device or resource busy */
#define GX_EEXIST      -9		/* File exists */
#define GX_ENOTDIR     -10		/* Not a directory */
#define GX_EISDIR      -11		/* Is a directory */
#define GX_EMFILE      -12		/* Too many open files */
#define GX_EFBIG       -13		/* File too large */
#define GX_ENOSPC      -14		/* No space left on device */
#define GX_EROFS       -15		/* Read-only file system */
#define GX_EAGAIN      -16		/* Resource temporarily unavailable */

#define GX_EGENERIC      -666                              /* Generic error */

#endif
