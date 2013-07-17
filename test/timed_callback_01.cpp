/*
 * Copyright (C) 2013 LAN Xingcan
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

#include <spin/spin.hpp>

int counter = 0;

using namespace spin;

class my_timer : public spin::timed_callback
{
private:
	int i;
public:
	my_timer(int i)
		: timed_callback(std::chrono::steady_clock::now()
				+ std::chrono::duration_cast<time_duration>(std::chrono::seconds(i))
      , [this](){
          callback();
      })
		, i(i)
	{  }

	void callback()
	{
		counter = i;
	}
};

int main()
{
	event_loop l;
	time_point tp = std::chrono::steady_clock::now();
	my_timer t1(1), t2(2), t3(3);
	l.post(t3);
	l.post(t2);
	l.post(t1);
	l.run();
	time_duration duration = std::chrono::steady_clock::now() - tp;
	if (duration < std::chrono::seconds(3))
		return 1;
	return 0;
}

