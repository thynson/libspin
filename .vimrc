fun! s:license_for_source()
	let license = [
\ "\/*",
\ " * Copyright (C) ",
\ " * All right reserved",
\ " *",
\ " * Permission to use, copy, modify, and/or distribute this software for any",
\ " * purpose with or without fee is hereby granted, provided that the above",
\ " * copyright notice and this permission notice appear in all copies.",
\ " *",
\ " * THE SOFTWARE IS PROVIDED \"AS IS\" AND THE AUTHOR DISCLAIMS ALL WARRANTIES",
\ " * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF",
\ " * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR",
\ " * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES",
\ " * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN",
\ " * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF",
\ " * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.",
\ " *\/"]
	call append(0, license)
	call cursor(2, col("."))
	" Write year
	let text = getline(".") . strftime("%Y")
	call setline(".", text)
	" Left cursor at the end of this line for name of copyright holder
	call cursor(line("."), col("$"))
endfun!

fun! s:c_style()
	set ts=2
	set st=2
	set sw=2
	set et
endfun!

fun! s:makefile_style()
	setlocal ts=4
	setlocal st=4
	setlocal sw=4
	setlocal noet
endfun!


autocmd FileType c call s:c_style()
autocmd FileType cpp call s:c_style()
autocmd BufNewFile *.cpp call s:license_for_source()
autocmd BufNewFile *.hpp call s:license_for_source()
autocmd FileType make call s:makefile_style()
autocmd FileType automake call s:makefile_style()

let g:ycm_confirm_extra_conf=0
