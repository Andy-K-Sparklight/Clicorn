#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Clicorn], [1.0.0], [Twilight_Princess_Sparkle@outlook.com])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign subdir-objects -Wall])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT64_T
AC_TYPE_SIZE_T

# Checks for library functions.
PKG_CHECK_MODULES([libs], [gtk+-3.0, webkit2gtk-4.0])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
