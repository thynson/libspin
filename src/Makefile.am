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

AM_CXXFLAGS=-Wall -Werror -Wextra -Wreorder
AM_CPPFLAGS=-fvisibility=hidden -D__BUILD_SPIN__=1
AM_LDFLAGS=-Xlinker --no-undefined -lpthread


if !SPIN_ENABLE_DEBUG
AM_CPPFLAGS+= -DNDEBUG
endif

libspin_ladir=$(includedir)/spin
lib_LTLIBRARIES=libspin.la
libspin_la_HEADERS=spin/utils.hpp\
				   spin/singleton.hpp\
				   spin/environment.hpp\
				   spin/functional.hpp\
				   spin/transform_iterator.hpp\
				   spin/spin_lock.hpp\
				   spin/event_loop.hpp\
				   spin/system.hpp\
				   spin/socket.hpp\
				   spin/intruse/list.hpp\
				   spin/intruse/rbtree.hpp\
				   spin/timer.hpp\
				   spin/task.hpp\
				   spin/service_template.hpp\
				   spin/thread_pool.hpp\
				   spin/poll.hpp


libspin_la_SOURCES=event_loop.cpp\
				   system.cpp\
				   socket.cpp\
				   intruse_rbtree.cpp\
				   timer.cpp\
				   thread_pool.cpp\
				   poll.cpp


