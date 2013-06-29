
set ts=2
set st=2
set sw=2
set et



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
	let text = getline(".") . strftime("%Y")
	call setline(".", text)
	call cursor(line("."), col("$"))
endfun!

autocmd BufNewFile *.cpp call s:license_for_source()
autocmd BufNewFile *.hpp call s:license_for_source()
