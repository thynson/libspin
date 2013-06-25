#include <spin/spin.hpp>

int counter = 0;

using namespace spin;

class my_timer : public spin::timer_event
{
private:
	int i;
public:
	my_timer(int i)
		: timer_event(std::chrono::steady_clock::now()
				+ std::chrono::duration_cast<time_duration>(std::chrono::seconds(i)))
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
	my_timer t1(1), t2(2), t3(3);
	l.post(t3);
	l.post(t2);
	l.post(t1);
	l.run();
}


