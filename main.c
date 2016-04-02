
#include "octtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include "force_calc.h"

int get_leaves(otree_t* tree,  otree_t** res, 
		       int curr_ind, int wanted){
	
	int tmp = 0;
	int accu = 0;
	if (tree->children[0] == NULL){
		if (tree->num_particles > 0){
			res[curr_ind] = tree;
			return 1;
		}else return 0;
	}
	for (int i = 0; i < 8; ++i){
		if (curr_ind == wanted){
			return accu; 
		}
		tmp = get_leaves (tree->children[i],res,curr_ind,wanted);
		curr_ind += tmp;
		accu += tmp;
	}
	return accu;
}

void test_inline_func(void){
	pmass_t a = {.pos =  {.x = 5.241, .y  = 9.211, .z = 1.23}};
	a.mass = 234.1;


	pmass_t b = {.pos = {.x = 15.231, .y  = 91.31, .z = 11.3}};
	b.mass = 21.34;

	point_t force;
	vector_gravity (&a, &b, &force);

	printf("(%lf, %lf, %lf)\n", force.x, force.y, force.z);
}

int main (int argc, char** argv){
	srand (time(NULL));
	otree_t* tree = otree_new (4096.0);
	floating_point x, y, z, mass;

	pmass_t part;
	otree_t* leaf;

//	test_inline_func();
//	assert(0);

	for (int i = 0; i < 10000; ++i){
		x = 4095.0 * (floating_point)rand()/(floating_point)RAND_MAX;
		y = 4095.0 * (floating_point)rand()/(floating_point)RAND_MAX;
		z = 4095.0 * (floating_point)rand()/(floating_point)RAND_MAX;
		mass = 1000 * (floating_point)rand()/(floating_point)RAND_MAX;
		
		part.pos.x = x, part.pos.y = y, part.pos.z = z;
		part.mass  = mass;

		leaf = otree_insert (tree,&part, 1); 
	}
	calculate_force(tree,tree);
	return 0;
}
