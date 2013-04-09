#include "../prioque.h"
#include <stdlib.h>


#define N 200

int main()
{
	int i;
	prioque_weight_t last = 0;
	prioque_node_t node[N];
	prioque_t q = prioque_create ();
	srand(0);

	for (i = 0; i < N; i++) {
		prioque_insert (q, node + i, rand() % 1000);
	}

	for (i = 0; i < N; i++) {
		prioque_update (q, node + i, rand() % 1000);
	}

	for (i = 0; i < N; i++) {
		prioque_node_t *p;
		prioque_weight_t weight;
		prioque_front (q, &p);
		prioque_get_node_weight (q, p, &weight);
		assert (last <= weight);
		last = weight;
		prioque_remove (q, p);
		printf ("%d\n", weight);
	}
	prioque_destroy (q);

}
