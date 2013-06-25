/*
 * Copyright (C) 2013  LAN Xingcan
 * All right reserved
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __SPIN_CONFIG_HPP_INCLUDED__
#define __SPIN_CONFIG_HPP_INCLUDED__

#ifdef __unix__
# if defined(PIC) && defined(__BUILD_SPIN__)
#   if defined(__GNUC__) || defined(__clang__)
#     define __SPIN_EXPORT__ __attribute__((visibility("default")))
#     define __SPIN_INTERNAL__ __attribute__((visibility("hidden")))
#   endif
# else
#   define __SPIN_EXPORT__
#   define __SPIN_INTERNAL__
# endif
#endif

#endif
