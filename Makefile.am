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
lib_LTLIBRARIES=libspin.la

libspin_ladir=$(includedir)/spin

libspin_la_LDFLAGS=@AM_LDFLAGS@ -lpthread -lrt \
				   -version-info $(subst .,:,$(API_VERSION))

libspin_la_CPPFLAGS=@AM_CPPFLAGS@
libspin_la_HEADERS=\
				   spin/config.h\
				   spin/spin.h\
				   spin/loop.h\
				   spin/timer.h\
				   spin/stream.h

libspin_la_SOURCES=\
				   loop.c\
				   timer.c\
				   prioque.h\
				   prioque.c\
				   linklist.h\
				   timespec.h\
				   poll-target.c\
				   stream.c\
				   spin.h

SUBDIRS=docs
