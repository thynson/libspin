#include <spin/spin.h>
#include <stdio.h>
#include <stdlib.h>

int counter = 0;
spin_timer_t timer;

int callback (void *param)
{
	printf ("counter = %d\n", counter++);
	if (counter == 5) {
		struct spin_itimespec x = {0, 0};
		spin_timer_ctl (timer, &x, NULL);
	}
	return 0;
}

int main()
{
	struct spin_itimespec x;
	x.initial = 2000;
	x.interval = 1000;
	spin_init ();
	spin_loop_t loop = spin_loop_create ();
	timer = spin_timer_create (loop, callback, NULL);
	spin_timer_ctl (timer, &x, NULL);

	spin_loop_run (loop);
	spin_loop_destroy (loop);
	spin_uninit ();
	return 0;
}

