#include <spin/spin.h>
#include <stdio.h>
#include <stdlib.h>

int counter = 0;
spin_timer_t timer_a;
spin_timer_t timer_b;

int callback_a (void *param)
{
	struct spin_itimespec x;
	spin_timer_ctl (timer_b, NULL, &x);
	printf ("counter = %d, timer_a : %u\n", counter++, x.initial);

	if (counter > 10) {
		x.initial = 0;
		x.interval = 0;
		spin_timer_ctl (timer_a, &x, NULL);
	}
	return 0;
}

int callback_b (void *param)
{
	struct spin_itimespec x;
	spin_timer_ctl (timer_a, NULL, &x);
	printf ("counter = %d, timer_b : %u\n", counter++, x.initial);

	if (counter > 10) {
		x.initial = 0;
		x.interval = 0;
		spin_timer_ctl (timer_b, &x, NULL);
	}
	return 0;
}

int main()
{
	struct spin_itimespec x;
	spin_init ();
	spin_loop_t loop = spin_loop_create ();
	timer_a = spin_timer_create (loop, callback_a, NULL);
	timer_b = spin_timer_create (loop, callback_b, NULL);

	x.initial = 2000;
	x.interval = 1000;
	spin_timer_ctl (timer_a, &x, NULL);

	x.initial = 500;
	x.interval = 1500;
	spin_timer_ctl (timer_b, &x, NULL);

	spin_loop_run (loop);

	counter = 0;

	x.initial = 1200;
	x.interval = 800;
	spin_timer_ctl (timer_a, &x, NULL);

	x.initial = 1000;
	x.interval = 700;
	spin_timer_ctl (timer_b, &x, NULL);

	spin_loop_run (loop);
	spin_loop_destroy (loop);
	spin_uninit ();
	return 0;
}

