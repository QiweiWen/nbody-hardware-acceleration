#include "octtree.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

char quadrant_result[2][2][2] = {{{0,4},{2,6}},{{1,5},{3,7}}};



static int out_of_bound (otree_t* node, point_t* pos){
	if (ABS(node->corner.x - pos->x) > node->side_len ||
		
		ABS(node->corner.y - pos->y) > node->side_len ||
		ABS(node->corner.z - pos->z) > node->side_len)
	{
		return 1;
	}else return 0;
}
//given a node and a coordinate, return which quadrant it is in
static char childnum (otree_t* node, point_t* pos){
	if (out_of_bound (node, pos)) return -1;	
	point_t centre = {.x = node->corner.x + node->side_len/2,
					  .y = node->corner.y + node->side_len/2,
					  .z = node->corner.z + node->side_len/2};
	
	char index = 0;
	char x_res = pos->x > centre.x,
		 y_res = pos->y > centre.y,
		 z_res = pos->z > centre.z;
	return quadrant_result[x_res][y_res][z_res];
}


static void otree_split (otree_t* node){
	for (int i = 0; i < 8; ++i){
		assert(node->children[i] == NULL);
		node->children[i] = otree_new((float_t)(node->side_len/2));
		node->children[i]->parent = node;
	}


	for (int i = 0; i < node->num_particles; ++i){
		char index = childnum (node, &node->particles[i].pos);
		otree_insert(node->children[index], &node->particles[i]);
	}
	node -> num_particles = 0;
}

static void otree_collapse(otree_t* node){
	//all these asserts:
	//if these invariants break then we should
	//fail noisily instead of silently
	for (int i = 0; i < 8; ++i){

		assert(node->children[i]->children[0] == NULL);
		assert(node->children[i]->num_particles == 
				   node->children[i]->total_particles);
		for (int j = 0; j < node->children[i]->num_particles; ++j){
			assert(node->num_particles <= OTREE_NODE_CAP);
			node->particles [node->num_particles ++] = 
				node->children[i]->particles[j];
		}
		free (node->children[i]);
		node->children[i] = NULL;
	}
	assert (node->num_particles == node->total_particles);
}

otree_t* otree_new(float_t side_len){
	otree_t* new_tree = (otree_t*)malloc(sizeof(otree_t));
	*new_tree = (otree_t){.centre_of_mass = (pmass_t){.mass = 0.0},
						  .side_len = side_len,
						  .corner = (point_t){0.0,0.0,0.0},
						  .num_particles = 0,
						  .total_particles = 0,  
						  .children = {NULL},
						  .parent = NULL};
	return new_tree;
}

otree_t* otree_insert (otree_t* tree, pmass_t* particle){
	tree->total_particles += 1;	
	
	if (tree->centre_of_mass.mass == (float_t)0){
		tree->centre_of_mass = *particle;
	}else{
		tree->centre_of_mass = centre_of_mass (&tree->centre_of_mass, particle);
	}
	
	if (tree->children[0] == NULL){
		//leaf
		if (tree->num_particles == OTREE_NODE_CAP){
			otree_split(tree);
			goto not_leaf_anymore;
		}else{
			tree->particles[tree->num_particles++] = *particle;
			return tree;
		}	
	}else{
		//not leaf
not_leaf_anymore:;
		char index = childnum (tree, &particle->pos);
		return otree_insert (tree->children[index], particle);
	}
}

void otree_free(otree_t* tree){
	for (int i = 0; i < 8; ++i){
		if (tree->children[i] != NULL){
			otree_free(tree->children[i]);
		}
	}
	free(tree);
}

void otree_rotate (otree_t* node, pmass_t* particle){
//TODO:
//finish the rotation code	
}

void otree_relocate(otree_t* tree, int i){
	assert(i < tree->num_particles);
	if (!out_of_bounds (tree, &tree->particles[i].pos)) {
		return;		
	}else{
		pmass_t part = tree->particles[i];
		for (int j = i + 1; j < tree->num_particles; ++j){
			tree->particles[j - 1] = tree->particles[j];
		}
		tree->num_particles--;
		tree->total_particles--;
		otree_rotate (tree, &part);
	}	
}


