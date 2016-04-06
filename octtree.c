#include "octtree.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

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


	for (int i = 0; i < node->num_particles; ++i){
		char index = childnum (node, &node->particles[i].pos);
		assert (index != -1);
		otree_insert(node->children[(int)index], &node->particles[i],1);
	}
	node -> num_particles = 0;
}

static int otree_collapse(otree_t* node,int old_child, int old_index){
	//printf("collapsing\n");
	//all these asserts:
	//if these invariants break then we should
	//fail noisily instead of silently
	assert (node->total_particles <= OTREE_NODE_CAP);
	int res = -1;
	for (int i = 0; i < 8; ++i){

		if (node->children[i]->children[0] == NULL){
			assert(node->children[i]->num_particles == 
					   node->children[i]->total_particles);
			for (int j = 0; j < node->children[i]->num_particles; ++j){
				if (res == -1){
					if (i == old_child && j == old_index){
						res = node->num_particles;
					}
				}
				assert(node->num_particles <= OTREE_NODE_CAP);
				node->particles [node->num_particles ++] = 
					node->children[i]->particles[j];
			}
			free (node->children[i]);
			node->children[i] = NULL;
		}else{
			otree_collapse(node->children[i],-1,-1);
			i--;
		}
	}
	assert (node->num_particles == node->total_particles);
	return res;
}

otree_t* otree_new(floating_point side_len){
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

otree_t* otree_insert (otree_t* tree, pmass_t* particle, int cal_com){
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
			tree->particles[tree->num_particles++] = *particle;
			return tree;
		}	
	}else{
		//not leaf
not_leaf_anymore:;
		char index = childnum (tree, &particle->pos);
		return otree_insert (tree->children[(int)index], particle, cal_com);
	}
}

void otree_free(otree_t* tree){
	for (int i = 0; i < 8; ++i){
		if (tree->children[i] != NULL){
			otree_free(tree->children[i]);
		}else continue;
	}
	free(tree);
}

otree_t* otree_relocate (otree_t* tree, int i, pmass_t* particle){
	assert (i < tree->num_particles);

	if (tree->children[0] == NULL){
		if (!out_of_bound (tree, &tree->particles[i].pos)){
			//leaf node, constraint met, job done
			return tree;
		}else{
			if (tree->parent == NULL) return NULL;	
			pmass_t part = tree->particles[i];
			for (int j = i + 1; j < tree->num_particles; ++j){
				tree->particles[j - 1] = tree->particles[j];
			}
			tree->total_particles--;
			tree->num_particles --;	
			return otree_relocate (tree->parent,-1,&part);
		}
	}else{
		if (!out_of_bound (tree, &particle->pos)){
			char index = childnum (tree, &particle->pos);
			return otree_insert (tree->children[(int)index],particle,0);
		}else{
			if (tree->parent == NULL){
				//we are at the root node and it's still out of bound
				//the thing has left the system
				return NULL;
			}
			tree->total_particles--;
			/*
			//BAD IDEA 
			if (tree->total_particles-- <= OTREE_NODE_CAP){
				//this should be exactly one level above leaf
				assert (tree->children[0]->children[0] == NULL);
				*com_origin = tree;
				otree_collapse (tree,-1,-1);
			}
			*/
			return otree_relocate (tree->parent, -1, particle);	
		}
	}
}



otree_t* otree_garbage_collect (otree_t* root){
	otree_t* parent = root->parent;	
	if (root->children[0] == NULL){
		if (parent == NULL)return root;
		if (parent->total_particles <= OTREE_NODE_CAP){
			otree_collapse(parent, -1, -1);
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
				otree_collapse (parent, -1, -1);
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
	int update_position = 1;
//	printf("AFFECTED NODES\n");
	//take away the influence of old particle from src and its ancestors
	for (;node_ptr != NULL;){
		if (update_position){
			new_centre = centre_of_mass (&node_ptr->centre_of_mass,
											   &adj_mass);
			centre_displ = dist_between_points_sqrd (&new_centre.pos,
										&node_ptr->centre_of_mass.pos);
			centre_displ = sqrt (centre_displ);


			if (centre_displ * COM_RESOLUTION <= node_ptr->side_len){
				update_position = 0;
			}
			node_ptr->centre_of_mass = new_centre;
		}else{
	
			node_ptr->centre_of_mass.mass += adj_mass.mass;
		}
//		printf("(0x%016x), %lf\n", node_ptr, node_ptr->centre_of_mass.mass);
		node_ptr = node_ptr->parent;
		
	}
//	printf("INSERTING BACK\n");
	//add in the influence of the new particle to dest and its ancestors
	adj_mass = *new_part;
	node_ptr = dst;
	update_position = 1;
	for (;node_ptr != NULL;){
		if (update_position){
			new_centre = centre_of_mass (&node_ptr->centre_of_mass,
											   &adj_mass);
			centre_displ = dist_between_points_sqrd (&new_centre.pos,
										&node_ptr->centre_of_mass.pos);
			centre_displ = sqrt (centre_displ);

			if (centre_displ * COM_RESOLUTION <= node_ptr->side_len){
				update_position = 0;
			}
			node_ptr->centre_of_mass = new_centre;
		}else{
		
			node_ptr->centre_of_mass.mass += adj_mass.mass;
		}
		
//		printf("(0x%016x), %lf\n", node_ptr, node_ptr->centre_of_mass.mass);
		node_ptr = node_ptr->parent;
	}
}

//in case we missed something
void check_constraints (otree_t* tree, int check_mass, int garbage_free){
	int is_leaf = (tree->children[0] == NULL);	
	floating_point mass = 0;
	if (check_mass){
		if (out_of_bound (tree, &tree->centre_of_mass.pos)){
			if (tree->num_particles != 0) assert(!"WTF");
		}
	}
	if (is_leaf){
		assert (tree->num_particles == tree->total_particles);
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
