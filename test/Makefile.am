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

AUTOMAKE_OPTIONS=foreign 1.7
ACLOCAL_AMFLAGS=-I build-aux/m4

AM_CPPFLAGS=-I$(top_srcdir)/src
AM_LDFLAGS=../src/libspin.la

check_PROGRAMS=test_singleton_01\
			   test_singleton_volatile_01\
			   test_main_loop_01\
			   test_main_loop_02\
			   test_main_loop_03\
			   test_intruse_list_01\
			   test_intruse_rbtree_01\
			   test_intruse_rbtree_02

TESTS=$(check_PROGRAMS)

test_singleton_volatile_01_SOURCES=singleton-volatile-01.cpp
test_singleton_01_SOURCES=singleton-01.cpp
test_main_loop_01_SOURCES=main_loop-01.cpp
test_main_loop_02_SOURCES=main_loop-02.cpp
test_main_loop_03_SOURCES=main_loop-03.cpp
test_intruse_list_01_SOURCES=intruse_list_01.cpp
test_intruse_rbtree_01_SOURCES=intruse_rbtree_01.cpp
test_intruse_rbtree_02_SOURCES=intruse_rbtree_02.cpp

