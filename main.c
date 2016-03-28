
#include "octtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

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

int main (int argc, char** argv){
	srand (time(NULL));
	otree_t* tree = otree_new (4096.0);
	floating_point x, y, z, mass;
	int randnum;
	pmass_t part;
	otree_t* leaf;
	for (int i = 0; i < 1000; ++i){
		x = 4096.0 * (floating_point)rand()/(floating_point)RAND_MAX;
		y = 4096.0 * (floating_point)rand()/(floating_point)RAND_MAX;
		z = 4096.0 * (floating_point)rand()/(floating_point)RAND_MAX;
		mass = 1000 * (floating_point)rand()/(floating_point)RAND_MAX;
		
		part.pos.x = x, part.pos.y = y, part.pos.z = z;
		part.mass  = mass;

		leaf = otree_insert (tree,&part, 1); 
	}
	
	check_constraints(tree);
	otree_t* leaves[1000];
	int num_leaves = get_leaves (tree, leaves, 0, 100);
	printf("%d\n", num_leaves);
	for (int j = 0; j < 100; ++j){
		printf("RELOCATING\n");
		get_leaves (tree, leaves, 0, 1);
		leaf = leaves[0];
		int ind = rand() % leaf->num_particles;


		x = 4096.0 * (floating_point)rand()/(floating_point)RAND_MAX;
		y = 4096.0 * (floating_point)rand()/(floating_point)RAND_MAX;
		z = 4096.0 * (floating_point)rand()/(floating_point)RAND_MAX;

		pmass_t old = leaf->particles[ind];

		leaf->particles[ind].pos.x = x;
		leaf->particles[ind].pos.y = y;
		leaf->particles[ind].pos.z = z;
		pmass_t new_mass = leaf->particles[ind];
	
		printf("mass of the moved particle: %lf\n", new_mass.mass);
		fflush(stdout);
		otree_t* new_leaf = otree_relocate (leaf, ind, NULL);

		printf("total mass should not change or something must be wrong\n");
		printf("old mass: %lf\n",tree->centre_of_mass.mass);

		otree_fix_com (leaf, new_leaf, &old, &new_mass);

		printf("new mass: %lf\n", tree->centre_of_mass.mass);

		assert (tree->total_particles == 1000);
		check_constraints(tree);
	}
	return 0;
}
