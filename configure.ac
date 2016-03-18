#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.4])
AC_INIT(minisatip, 0.5, catalinii@yahoo.com)
AC_CONFIG_SRCDIR([minisatip.h])
# AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lrt':
AC_CHECK_LIB([rt], [main])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([bzero clock_gettime dup2 gethostbyname gettimeofday inet_ntoa memset munmap socket strchr strerror strncasecmp strstr])

FM="\nminisatip configuration\n\n"
linux_dvb="Linux DVB:\t\t\t\t "
dvb_csa="DVBCSA (needs libdvbcsa):\t\t "
openssl="OpenSSL (AES as part of DVBAPI):\t "
dvb_ca="Common Interface (needs DVBEN50221):\t "
emb="Embedded system:\t\t\t "
netcv="Netceiver support:\t\t\t "
satipc="SatIP Client:\t\t\t\t "
static="Static:\t\t\t\t\t "


AC_MSG_NOTICE([ == Checking LINUX DVB == ])
AC_CHECK_HEADERS([linux/dvb/frontend.h],[LINUXDVB=1],[LINUXDVB=0])
AC_ARG_ENABLE([linuxdvb],
  [AS_HELP_STRING([--disable-linuxdvb],
  [disable linux dvb support])],
  [LINUXDVB=0],
  [LINUXDVB="$LINUXDVB"])

if test "$LINUXDVB" = "1"; then
FM="$FM\n$linux_dvb enabled"
else
FM="$FM\n$linux_dvb disabled"
fi

AC_MSG_NOTICE([ == Checking DVBEN50221 == ])
AC_CHECK_HEADERS([libdvben50221/en50221_app_dvb.h],[DVBCA=1],[DVBCA=0])
AC_ARG_ENABLE([dvbca],
  [AS_HELP_STRING([--disable-dvbca],
  [disable dvbca support (decrypting using CI)])],
  [DVBCA=0],
  [DVBCA="$DVBCA"])

if test "$DVBCA" = "1"; then
FM="$FM\n$dvb_ca enabled"
else
FM="$FM\n$dvb_ca disabled"
fi

AC_MSG_NOTICE([ == Checking OpenSSL == ])
AC_CHECK_HEADERS([openssl/aes.h],[DVBAES=1],[DVBAES=0])
AC_ARG_ENABLE([dvbaes],
  [AS_HELP_STRING([--disable-dvbaes],
  [disable AES support (decrypting using openssl)])],
  [DVBAES=0],
  [DVBAES="$DVBAES"])

if test "$DVBAES" = "1"; then
FM="$FM\n$openssl enabled"
else
FM="$FM\n$openssl disabled"
fi


AC_CHECK_HEADERS([execinfo.h],[EMBEDDED=0],[EMBEDDED=1])
if test "$EMBEDDED" = "1"; then
FM="$FM\n$emb enabled"
else
FM="$FM\n$emb disabled"
fi

AC_MSG_NOTICE([ == Checking DVBCSA == ])
AC_CHECK_HEADERS([dvbcsa/dvbcsa.h],[DVBCSA=1],[DVBCSA=0])
AC_ARG_ENABLE([dvbcsa],
  [AS_HELP_STRING([--disable-dvbcsa],
  [disable dvbcsa support (decrypting with dvbcsa)])],
  [DVBCSA=0],
  [DVBCSA="$DVBCSA"])

if test "$DVBCSA" = "1"; then
FM="$FM\n$dvb_csa enabled"
else
FM="$FM\n$dvb_csa disabled"
fi

CFLAGS="-I. -I../vdr-mcli-plugin/mcast/client -I../vdr-mcli-plugin/mcast/common `xml2-config --cflags`"
CPPFLAGS="-I. -I../vdr-mcli-plugin/mcast/client -I../vdr-mcli-plugin/mcast/common `xml2-config --cflags`"

AC_MSG_NOTICE([ == Checking NETCVCLIENT == ])
AC_CHECK_HEADERS([netceiver.h],[NETCVCLIENT=1],[NETCVCLIENT=0])
AC_ARG_ENABLE([netcv],
  [AS_HELP_STRING([--disable-netceiver],
  [disable NETCEIVER support])],
  [NETCVCLIENT=0],
  [NETCVCLIENT="$NETCVCLIENT"])

if test "$NETCVCLIENT" = "1"; then
FM="$FM\n$netcv enabled"
else
FM="$FM\n$netcv disabled"
fi


AC_ARG_ENABLE([satipc],
  [AS_HELP_STRING([--disable-satipc],
  [disable satip client support])],
  [SATIPCLIENT=0],
  [SATIPCLIENT=1])
if test "$SATIPCLIENT" = "1"; then
FM="$FM\n$satipc enabled"
else
FM="$FM\n$satipc disabled"
fi

AC_ARG_ENABLE([static],
  [AS_HELP_STRING([--enable-static],
  [enable static compilation])],
  [STATIC=1],
  [STATIC=0])
if test "$STATIC" = "1"; then
FM="$FM\n$static enabled"
else
FM="$FM\n$static disabled"
fi


AC_SUBST(LINUXDVB)
AC_SUBST(DVBCSA)
AC_SUBST(DVBCA)
AC_SUBST(DVBAES)
AC_SUBST(EMBEDDED)
AC_SUBST(NETCVCLIENT)
AC_SUBST(SATIPCLIENT)
AC_SUBST(STATIC)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

echo -e "$FM\n"
