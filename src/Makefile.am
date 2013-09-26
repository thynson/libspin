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

libspin_ladir=$(includedir)/spin
lib_LTLIBRARIES=libspin.la
libspin_la_HEADERS=spin/utils.hpp\
				   spin/environment.hpp\
				   spin/spin_lock.hpp\
				   spin/main_loop.hpp\
				   spin/handle.hpp

libspin_la_SOURCES=main_loop.cpp\
				   handle.cpp


