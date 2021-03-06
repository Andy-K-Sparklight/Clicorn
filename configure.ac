#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Clicorn], [1.0.0], [Twilight_Princess_Sparkle@outlook.com])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([include/config.h])
AC_CANONICAL_HOST

build_linux=no
build_windows=no

case "${host_os}" in
    linux*)
        build_linux=yes
        PKG_CHECK_MODULES([libs], [gtk+-3.0, webkit2gtk-4.0, libcurl])
        ;;
    cygwin*|mingw*)
        build_windows=yes
        ;;
    *)
        AC_MSG_ERROR(["OS $host_os is not supported"])
        ;;
esac

AM_CONDITIONAL([LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([WINDOWS], [test "$build_windows" = "yes"])

AM_INIT_AUTOMAKE([foreign subdir-objects -Wall])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC


# Checks for header files.
AC_CHECK_HEADERS([fcntl.h float.h limits.h locale.h stddef.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h utime.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([getcwd localeconv memset mkdir strcasecmp strchr strncasecmp strstr sysinfo utime])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
