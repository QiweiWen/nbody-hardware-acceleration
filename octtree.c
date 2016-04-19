#include "octtree.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "dllist.h"
#include <string.h>

char quadrant_result[2][2][2] = {{{0,4},{2,6}},{{1,5},{3,7}}};



static int out_of_bound (otree_t* node, point_t* pos){
	if (pos->x < node->corner.x || pos->y < node->corner.y 
	   ||pos->z < node->corner.z) return 1;
	if (pos->x - node->corner.x >= node->side_len ||
		
		pos->y - node->corner.y >= node->side_len ||
		pos->z - node->corner.z >= node->side_len)
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
	

	char x_res = pos->x >= centre.x,
		 y_res = pos->y >= centre.y,
		 z_res = pos->z >= centre.z;

	return quadrant_result[(int)x_res][(int)y_res][(int)z_res];
}

static point_t get_corner (otree_t* node, char index){
	point_t centre = {.x = node->corner.x + node->side_len/2,
					  .y = node->corner.y + node->side_len/2,
					  .z = node->corner.z + node->side_len/2};
	point_t corner;
	if (index > 3){
	   	corner.z = centre.z;
	}else{
		corner.z = node->corner.z;
	}
	if ((index % 2) == 0){
		corner.x = node->corner.x;
	}else{
		corner.x = centre.x;
	}
	if ((index % 4) < 2){
		corner.y = node->corner.y;
	}else{
		corner.y = centre.y;
	}
	return corner;
}
	
static void otree_split (otree_t* node){
	for (int i = 0; i < 8; ++i){
		assert(node->children[i] == NULL);
		node->children[i] = otree_new((floating_point)(node->side_len/2));
		node->children[i]->parent = node;
		node->children[i]->corner = get_corner (node, i);
	}


	dlnode_t* curr = node->particles->first,
			* next;
	otree_t* the_tree;
	char index;
	while (curr != NULL){
		next = curr->next;
		index = childnum (node, &((pmass_t*)(curr->key))->pos);
		the_tree = node->children[(int)index];
	

		otree_insert (the_tree, curr, curr->key, 1);
		curr = next;
	}	

	node -> num_particles = 0;
	memset (node->particles, 0, sizeof (dllist_t));
}

static void otree_collapse(otree_t* node){
	//dbprintf("collapsing\n");
	//all these asserts:
	//if these invariants break then we should
	//fail noisily instead of silently
	printf("collapsing %16lx\n", (unsigned long)node);
	printf ("corner (%lf, %lf, %lf), com (%lf, %lf, %lf), %lf kg\n", 
			node->corner.x, node->corner.y, node->corner.z,
			node->centre_of_mass.pos.x, node->centre_of_mass.pos.y,node->centre_of_mass.pos.z, node->centre_of_mass.mass);
	assert (node->total_particles <= OTREE_NODE_CAP);

	for (int i = 0; i < 8; ++i){

		if (node->children[i]->children[0] == NULL){
			assert(node->children[i]->num_particles == 
					   node->children[i]->total_particles);
			//stick the childs list to ours	
			append_dllist (node->particles, node->children[i]->particles);	
			node->num_particles += node->children[i]->num_particles;	
			free (node->children[i]->particles);
			free (node->children[i]);
			node->children[i] = NULL;
		}else{
			otree_collapse(node->children[i]);
			i--;
		}
	}
	assert (node->num_particles == node->total_particles);

}

otree_t* otree_new(floating_point side_len){
	otree_t* new_tree = (otree_t*)malloc(sizeof(otree_t));
	*new_tree = (otree_t){.centre_of_mass = (pmass_t){.mass = 0.0},
						  .side_len = side_len,
						  .corner = (point_t){0.0,0.0,0.0},
						  .num_particles = 0,
						  .total_particles = 0,  
						  .children = {NULL},
						  .parent = NULL,
						  .particles = new_dllist ()};
	return new_tree;
}

otree_t* otree_insert (otree_t* tree, dlnode_t* lnk, pmass_t* particle, int cal_com){
	tree->total_particles += 1;	
	
	assert (!out_of_bound(tree, &particle->pos));	
	if (cal_com){
		if (tree->centre_of_mass.mass == (floating_point)0){
			tree->centre_of_mass = *particle;
		}else{
			tree->centre_of_mass = centre_of_mass (&tree->centre_of_mass, 
												   particle);
		}
	}
	if (tree->children[0] == NULL){
		//leaf
		if (tree->num_particles == OTREE_NODE_CAP){
			otree_split(tree);
			goto not_leaf_anymore;
		}else{
			//tree->particles[tree->num_particles++] = *particle;
			tree->num_particles++;
			if (lnk){
				insert_dllist_link (tree->particles, lnk);	
			}else{
				insert_dllist (tree->particles, particle);
			}			
			return tree;
		}	
	}else{
		//not leaf
not_leaf_anymore:;
		char index = childnum (tree, &particle->pos);
		return otree_insert (tree->children[(int)index], lnk, particle, cal_com);
	}
}

void otree_free(otree_t* tree){
	for (int i = 0; i < 8; ++i){
		if (tree->children[i] != NULL){
			otree_free(tree->children[i]);
		}else continue;
	}
	delete_dllist (tree->particles, free);
	free(tree);
}

static void  __attribute__((unused)) print_vector (point_t* vec){
	dbprintf("(%.20lf, %.20lf, %.20lf)\n", vec->x, vec->y, vec->z);
}

otree_t* otree_relocate (otree_t* tree, dlnode_t* particle){
	

	if (tree->children[0] == NULL){
		if (!out_of_bound (tree, 
						   &((pmass_t*)(particle->key))->pos ))
		{
			//leaf node, constraint met, job done
			return tree;
		}else{
			tree->total_particles--;
			tree->num_particles --;	
			if (tree->parent == NULL){
				free (particle->key);
				free (particle);
			   	return NULL;
			}
			dllist_delete_node (tree->particles, particle, 0, NULL);
			return otree_relocate (tree->parent,particle);
		}
	}else{
		if (!out_of_bound (tree, &((pmass_t*)(particle->key))->pos ) ){
			char index = childnum (tree, &((pmass_t*)(particle->key))->pos);
			return otree_insert (tree->children[(int)index],particle,particle->key,0);
		}else{
			if (tree->parent == NULL){
				//we are at the root node and it's still out of bound
				//the thing has left the system
				return NULL;
			}
			tree->total_particles--;
			return otree_relocate (tree->parent, particle);	
		}
	}
}



otree_t* otree_garbage_collect (otree_t* root){
	otree_t* parent = root->parent;	
	if (root->children[0] == NULL){
		if (parent == NULL)return root;
		if (parent->total_particles <= OTREE_NODE_CAP){
			otree_collapse(parent);
			//"root" not valid anymore
			return NULL;
		}else{
			return root;
		}
	}else{
		int i_am_a_leaf = 0;
		for (int i = 0; i < 8; ++i){
			i_am_a_leaf = 
				(otree_garbage_collect(root->children[i]) == NULL);
			if (i_am_a_leaf) break;
		}
		if (i_am_a_leaf){
			if (parent == NULL) return root;	
			if (parent->total_particles <= OTREE_NODE_CAP){
				otree_collapse (parent);
				return NULL;
			}else{
				return root;
			}
		}else return root;
	}
}

void otree_fix_com (otree_t* src, otree_t* dst, pmass_t* old_part, 
					pmass_t* new_part)
{
	otree_t* node_ptr = src;
	pmass_t adj_mass = *old_part;
	adj_mass. mass *= -1;
	pmass_t new_centre;
	floating_point centre_displ;
	
	floating_point mass_diff;
	for (;node_ptr != NULL;){
		
		mass_diff = ABS(node_ptr->centre_of_mass.mass - old_part->mass);
		
		if (mass_diff < MIN_MASS){
			memset (&node_ptr->centre_of_mass, 0, sizeof (pmass_t));
			node_ptr = node_ptr->parent;
			continue;
		}
		
		new_centre = centre_of_mass (&node_ptr->centre_of_mass,
										   &adj_mass);
		centre_displ = dist_between_points_sqrd (&new_centre.pos,
									&node_ptr->centre_of_mass.pos);
		centre_displ = sqrt (centre_displ);
		node_ptr->centre_of_mass = new_centre;
	
		node_ptr = node_ptr->parent;
		
	}
	adj_mass = *new_part;
	node_ptr = dst;

	for (;node_ptr != NULL;){
		
		if (node_ptr->centre_of_mass.mass < MIN_MASS){
			node_ptr->centre_of_mass = adj_mass;
			node_ptr = node_ptr->parent;
			continue;
		}
		new_centre = centre_of_mass (&node_ptr->centre_of_mass,
										   &adj_mass);
		centre_displ = dist_between_points_sqrd (&new_centre.pos,
									&node_ptr->centre_of_mass.pos);
		centre_displ = sqrt (centre_displ);
		node_ptr->centre_of_mass = new_centre;

		node_ptr = node_ptr->parent;
	}
}

//in case we missed something
void check_constraints (otree_t* tree, int check_mass, int garbage_free){
	int is_leaf = (tree->children[0] == NULL);	
	floating_point mass = 0;
	if (check_mass){
		if (tree->centre_of_mass.mass >= MIN_MASS){
			assert (tree->centre_of_mass.pos.x >= 0);	
			assert (tree->centre_of_mass.pos.y >= 0);
			assert (tree->centre_of_mass.pos.z >= 0);

			if (out_of_bound (tree, &tree->centre_of_mass.pos)){
				assert(!"WTF");
			}
		}
	}
	if (is_leaf){
		assert (tree->num_particles == tree->total_particles);
		assert (tree->particles->num == tree->num_particles);
	}else{
		assert (tree->num_particles == 0);
		if (garbage_free) assert (tree->total_particles >= OTREE_NODE_CAP);
		int sum = 0;
		mass = 0;
		for (int i = 0; i < 8; ++i){
			check_constraints (tree->children[i], check_mass,garbage_free);
			sum += tree->children[i]->total_particles;
			mass += tree->children[i]->centre_of_mass.mass;
		}
		assert (sum == tree->total_particles);
	
//		if (check_mass) assert ((ABS(mass) - ABS(tree->centre_of_mass.mass))/ABS(mass) < 0.01);
	}

}
