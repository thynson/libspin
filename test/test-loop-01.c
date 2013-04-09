#include <spin/spin.h>
#include <stdio.h>
#include <stdlib.h>


int main()
{
	spin_loop_t loop = spin_loop_create ();
	spin_loop_run (loop);
	spin_loop_destroy (loop);
	return 0;
}

