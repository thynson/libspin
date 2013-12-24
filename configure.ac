#
# Copyright (C) 2013 LAN Xingcan
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

AC_PREREQ([2.67])
AC_INIT([libspin], [0.0.1])

AC_CONFIG_MACRO_DIR([build-aux/m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([1.11 dist-xz])
AC_CONFIG_MACRO_DIRS([m4])

AC_ARG_ENABLE(debug, AS_HELP_STRING([--enable-debug],
	[Build with debug support (default: disabled)]),
	[spin_enable_debug=$enableval], [spin_enable_debug=no])
AM_CONDITIONAL(SPIN_ENABLE_DEBUG, [ test "x$spin_enable_debug" != xno ])


AC_PROG_CC([clang gcc])
AC_PROG_CXX([clang++ g++])
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX_11([noext],[mandatory])

AM_PROG_LIBTOOL
LT_INIT
AM_SILENT_RULES([yes])

BOOST_REQUIRE([1.48])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([example/Makefile])
AC_CONFIG_FILES([test/Makefile])
AC_CONFIG_FILES([docs/Makefile])
AC_OUTPUT



