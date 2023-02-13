#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.4])
AC_INIT(minisatip, 1.1, https://github.com/catalinii/minisatip)
AC_CONFIG_SRCDIR([src/minisatip.h])
#AC_CONFIG_HEADERS([config.h])

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
CI="Common Interface (CI)\t\t\t "
dvb_ca="Common Interface (needs DVBEN50221):\t "
emb="Embedded system:\t\t\t "
netcv="Netceiver support:\t\t\t "
satipc="SatIP Client:\t\t\t\t "
static="Static:\t\t\t\t\t "
dvbapi="dvbapi:\t\t\t\t\t "
axe="axe:\t\t\t\t\t "
enigma="enigma:\t\t\t\t\t "
LINUXDVB=0
DVBCA=0
DVBCSA=0
NETCVCLIENT=0
SATIPCLIENT=0
STATIC=0
AXE=0
DVBAPI=0

AC_ARG_ENABLE([linuxdvb],
  [AS_HELP_STRING([--disable-linuxdvb],
  [disable linux dvb support])])

AS_IF([test "x$enable_linuxdvb" != "xno"], [
AC_MSG_NOTICE([ == Checking LINUX DVB == ])
AC_CHECK_HEADERS([linux/dvb/frontend.h],[LINUXDVB=1],[LINUXDVB=0])
LINUXDVB=1
]) 

if test "$LINUXDVB" = "1"; then
FM="$FM\n$linux_dvb enabled"
else
FM="$FM\n$linux_dvb disabled"
fi

AC_ARG_ENABLE([dvbca],
  [AS_HELP_STRING([--disable-dvbca],
  [disable Common Interface (CI) support (relies on openssl)])])

AS_IF([test "x$enable_dvbca" != "xno"], [
AC_MSG_NOTICE([ == Checking OpenSSL == ])
AC_CHECK_HEADERS([openssl/aes.h],[DVBCA=1],[DVBCA=0])
])

if test "$DVBCA" = "1"; then
FM="$FM\n$CI enabled"
else
FM="$FM\n$CI disabled"
fi

AC_MSG_NOTICE([ == Checking Embedded == ])
AC_CHECK_FUNCS([backtrace],[EMBEDDED=0],[EMBEDDED=1])
if test "$EMBEDDED" = "1"; then
FM="$FM\n$emb enabled"
else
FM="$FM\n$emb disabled"
fi

AC_ARG_ENABLE([dvbcsa],
  [AS_HELP_STRING([--disable-dvbcsa],
  [disable dvbcsa support (decrypting with dvbcsa)])])

AS_IF([test "x$enable_dvbcsa" != "xno"], [
AC_MSG_NOTICE([ == Checking DVBCSA == ])
AC_CHECK_HEADERS([dvbcsa/dvbcsa.h],[DVBCSA=1],[DVBCSA=0])
])

if test "$DVBCSA" = "1"; then
FM="$FM\n$dvb_csa enabled"
else
FM="$FM\n$dvb_csa disabled"
fi


AC_ARG_ENABLE([netcv],
  [AS_HELP_STRING([--disable-netcv],
  [disable NETCEIVER support])])

AS_IF([test "x$enable_netcv" != "xno"], [
AC_MSG_NOTICE([ == Checking NETCVCLIENT == ])

mcli_dir="../../vdr-mcli-plugin"
xml2_dir="/usr/include/libxml2"

AC_ARG_WITH([mcli],
  [AS_HELP_STRING([--with-mcli],
  [specify vdr-mcli-plugin directory default=../../vdr-mcli-plugin])],
  [mcli_dir=$with_mcli])

AC_ARG_WITH([xml2],
  [AS_HELP_STRING([--with-xml2],
  [specify libxml2 directory default=/usr/include/libxml2])],
  [xml2_dir=$with_xml2])
  
CFLAGS="-I. -I$mcli_dir/mcast/client -I$mcli_dir/mcast/common -L$mcli_dir/mcast/client -I$xml2_dir"
CPPFLAGS="-I. -I$mcli_dir/mcast/client -I$mcli_dir/mcast/common -L$mcli_dir/mcast/client -I$xml2_dir"

cd src
AC_CHECK_HEADERS([netceiver.h],[NETCVCLIENT=1],[NETCVCLIENT=0])
cd ..
])

if test "$NETCVCLIENT" = "1"; then
FM="$FM\n$netcv enabled"
else
FM="$FM\n$netcv disabled"
CFLAGS=
CPPFLAGS=
fi


AC_ARG_ENABLE([satipc],
  [AS_HELP_STRING([--disable-satipc],
  [disable satip client support])])

AS_IF([test "x$enable_satipc" != "xno"], [
SATIPCLIENT=1
])

if test "$SATIPCLIENT" = "1"; then
FM="$FM\n$satipc enabled"
else
FM="$FM\n$satipc disabled"
fi

AC_ARG_ENABLE([static],
  [AS_HELP_STRING([--enable-static],
  [enable static compilation])])

AS_IF([test "x$enable_static" = "xyes"], [
STATIC=1
])
  
if test "$STATIC" = "1"; then
FM="$FM\n$static enabled"
else
FM="$FM\n$static disabled"
fi

AC_ARG_ENABLE([dvbapi],
  [AS_HELP_STRING([--disable-dvbapi],
  [disable dvbapi protocol support])])

AS_IF([test "x$enable_dvbapi" != "xno"], [
DVBAPI=1
])

if test "$DVBAPI" = "1"; then
FM="$FM\n$dvbapi enabled"
else
FM="$FM\n$dvbapi disabled"
fi


AC_ARG_ENABLE([axe],
  [AS_HELP_STRING([--enable-axe],
  [enable compilation for axe: Inverto IDL400])])

AS_IF([test "x$enable_axe" = "xyes"], [
AXE=1
])
  
if test "$AXE" = "1"; then
FM="$FM\n$axe enabled"
else
FM="$FM\n$axe disabled"
fi

ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -fstrict-flex-arrays"

AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],
  [[int i = 1; (void) i;]])],
  [enable_strict_flex_array=1], [enable_strict_flex_array=0])

AS_IF([test $enable_strict_flex_array -eq 0],
  [CFLAGS="$ac_save_CFLAGS"])

ac_save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -fsanitize=bounds -fsanitize-trap=all"

AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],
  [[int i = 1; (void) i;]])],
  [enable_bounds_check=1], [enable_bounds_check=0])

AS_IF([test $enable_bounds_check -eq 0],
  [CFLAGS="$ac_save_CFLAGS"])

FM="$FM\nCCFLAGS\t\t$CFLAGS"


AC_SUBST(LINUXDVB)
AC_SUBST(DVBCSA)
AC_SUBST(DVBCA)
AC_SUBST(EMBEDDED)
AC_SUBST(NETCVCLIENT)
AC_SUBST(SATIPCLIENT)
AC_SUBST(STATIC)
AC_SUBST(DVBAPI)
AC_SUBST(AXE)

AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([tests/Makefile])
AC_CONFIG_FILES([Makefile])

AC_OUTPUT

echo -e "$FM\n"
