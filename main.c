
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
	pmass_t* part;
	otree_t* leaf;
	printf ("%d\n", OTREE_NODE_CAP);
	
	for (int i = 0; i < 1000; ++i){
		x = 4096.0 * (floating_point)rand()/(floating_point)RAND_MAX;
		y = 4096.0 * (floating_point)rand()/(floating_point)RAND_MAX;
		z = 4096.0 * (floating_point)rand()/(floating_point)RAND_MAX;
		mass = 1000 * (floating_point)rand()/(floating_point)RAND_MAX;
		
		part = malloc (sizeof (pmass_t));
		part->pos.x = x, part->pos.y = y, part->pos.z = z;
		part->mass  = mass;

		leaf = otree_insert (tree,NULL,part, 1); 
	}
	
	check_constraints(tree,1,1);
	
	

	for (int j = 0; j < 10000; ++j){
		//printf("RELOCATING\n");
		get_leaves (tree, &leaf, 0, 1);
	
		int ind = rand() % leaf->num_particles;


		x = 4096.0 * (floating_point)rand()/(floating_point)RAND_MAX;
		y = 4096.0 * (floating_point)rand()/(floating_point)RAND_MAX;
		z = 4096.0 * (floating_point)rand()/(floating_point)RAND_MAX;

		pmass_t old = *(pmass_t*)(leaf->particles->first->key);
		pmass_t* old_ptr = (leaf->particles->first->key);
		old_ptr->pos.x = x;
		old_ptr->pos.y = y;
		old_ptr->pos.z = z;
		
		pmass_t new_mass = *(pmass_t*)(leaf->particles->first->key);
		printf ("==== %d\n", leaf->num_particles);	
		printf ("leaf: %016x, pos, mass: (%lf, %lf, %lf), %lf\n",
			leaf, old.pos.x, old.pos.y, old.pos.z, old.mass);	
		printf ("old com: pos, mass: (%lf, %lf, %lf), %lf",
				leaf->centre_of_mass.pos.x,leaf->centre_of_mass.pos.y,leaf->centre_of_mass.pos.z, leaf->centre_of_mass.mass);
		printf ("====\n");	

		otree_t* new_leaf = otree_relocate (leaf, leaf->particles->first);

		check_constraints(tree,0,0);
		//if (new_leaf != leaf){
		//	printf ("MOVED\n");
			otree_fix_com (leaf, new_leaf, &old, &new_mass);
		//}else{
		//	printf ("NOT MOVED\n");
		//}
		printf("new mass: %lf\n", tree->centre_of_mass.mass);
		check_constraints(tree,1,0);
	

	

		assert (tree->total_particles == 1000);
	
	}
	assert(tree == otree_garbage_collect (tree));
		check_constraints(tree,1,1);
		
	return 0;
}
