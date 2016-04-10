#include "octtree.h"
#include "point_mass.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "list.h"
#include <stdint.h>

static void print_vector (point_t* vec){
	dbprintf("(%lf, %lf, %lf)\n", vec->x, vec->y, vec->z);
}

static void print_pmass (void* data){
	pmass_t* part = (pmass_t*)data;
	dbprintf("===\n");
	dbprintf ("mass %lf\n",part->mass);
	dbprintf("coord: ");print_vector (&part->pos);
	dbprintf("acc: ");print_vector (&part->acc);	
	dbprintf("vel: ");print_vector (&part->vel);

}

//node: the node for which we are constructing the ilist
//
//target: the potential node to add to origin's ilist
static inline int far_far_away(otree_t* node, otree_t* target){
	point_t tmp	= {.x = node->corner.x + node->side_len/2,
				   .y = node->corner.y + node->side_len/2,
				   .z = node->corner.z + node->side_len/2};
	
	char x_res = target->centre_of_mass.pos.x > tmp.x,
		 y_res = target->centre_of_mass.pos.y > tmp.y,
		 z_res = target->centre_of_mass.pos.z > tmp.z;
	tmp = node->corner;
	//find the farthest corner in "node" from target
	//so target satisfies the barnes-hut criterion for all
	//particles under "node"
	if (!x_res){
		tmp.x += node->side_len;	
	}
	if (!y_res){
		tmp.y += node->side_len;
	}
	if (!z_res){
		tmp.z += node->side_len;
	}
	floating_point width = target->side_len,
				   dist  = sqrt(dist_between_points_sqrd(
							   &tmp, &target->centre_of_mass.pos));
	/*
	if (width/dist > BH_THETA){
		dbprintf("=================\n");
		dbprintf("corner: (%lf, %lf, %lf)\n", tmp.x, tmp.y, tmp.z);
		dbprintf("distant COM:");
		print_pmass ((void*)&target->centre_of_mass);
		dbprintf("width %lf, dist %lf, theta %lf\n", width, dist, width/dist);		
	}
	*/
	return (width/dist <= BH_THETA);
}

static int make_interaction_list (int barnes_hut, otree_t* currnode, 
								   otree_t* origin,
								   List* ilist,int* list_is_empty)
{
	int res = 0;
	if (*ilist == NULL){
		*ilist = newList();
	}
	if (currnode == origin) return 0;
	if (currnode->centre_of_mass.mass < 0.1) return 0;	
	if (currnode->children[0] == NULL){
		//leaf
		//add all particles
	
		for (int i = 0; i < currnode->num_particles; ++i){
			pmass_t* part_heap = malloc(sizeof(pmass_t));
			*part_heap = currnode->particles[i];
			if (*list_is_empty){
				add_list (part_heap, *ilist);	
				*list_is_empty = 0;
			}else{
				*ilist = list_push (*ilist,part_heap);	
			}
			++res;
		}
	}else{
		//not leaf
		//is the node far enough?
		int go_further = !barnes_hut || !far_far_away(origin, currnode);
		if (go_further){
			for (int i = 0; i < 8; ++i){
				res += make_interaction_list (barnes_hut, currnode->children[i],origin,
									  ilist,list_is_empty);
			}
		}else{
			pmass_t* part_heap = malloc(sizeof(pmass_t));
			*part_heap = currnode->centre_of_mass;
			if (*list_is_empty){
				add_list (part_heap, *ilist);
				*list_is_empty = 0;	
			}else{
				*ilist = list_push (*ilist, part_heap);
			}
			++res;
		}	
	}
	return res;
}
static int yes (void* data) {
	return 1;
}

uint64_t direct_sum_times = 0;
uint64_t direct_sum_total_len = 0;

uint64_t group_times = 0;
uint64_t group_sum_total_len = 0;

uint64_t sum_ilist_count = 0;

static void __attribute__ ((noinline)) sum_interaction_list (otree_t* node, List ilist)  {
	++sum_ilist_count;
	List head = ilist;
	pmass_t* part;
	point_t  force;
	node->centre_of_mass.acc.x = 0;	
	node->centre_of_mass.acc.y = 0;
	node->centre_of_mass.acc.z = 0;
	while (head != NULL){
		part = (pmass_t*)list_getKey(head);
		vector_gravity (&node->centre_of_mass,part, &force);
		vector_add (&(node->centre_of_mass.acc),&force);
		head = list_pop(head);
	}	
}

static void direct_sum_force (otree_t* root, otree_t* currnode, List ilist){
	if (currnode->children[0] == NULL){
		//apply effect of ilist on every particle
		for (int i = 0; i < currnode->num_particles; ++i){
		//	dbprintf("%d\n", i);
			List head = ilist;
			pmass_t* part;
			point_t force;
			currnode->particles[i].acc.x = 0;
			currnode->particles[i].acc.y = 0;
			currnode->particles[i].acc.z = 0;

			while (head != NULL){


				part = (pmass_t*)head->key;
				
				if (vector_equal (&part->pos,&(currnode->particles[i].pos))){
					head = head->next;
				   	continue;
				}	
				vector_gravity (&currnode->particles[i],part, &force);
				vector_add (&(currnode->particles[i].acc), &force);
				head = head->next;
			}
			vector_add (&(currnode->particles[i].acc),&(root->centre_of_mass.acc));
		}	
	}else{
		for (int i = 0; i < 8; ++i){
			direct_sum_force (root, currnode->children[i], ilist);
		}
	}
}

static void direct_sum (otree_t* node){
	assert(node -> total_particles <= GROUP_SIZE);
	//make a global ilist
	List ilist = NULL;
	int list_is_empty = 1;
	direct_sum_times += 1;
	direct_sum_total_len += make_interaction_list (0, node, NULL, &ilist, &list_is_empty);
	direct_sum_force (node, node, ilist);
	destroyList (ilist, free);
}

void calculate_force (otree_t* root,otree_t* node){
	assert(node);

	if (node->total_particles < GROUP_SIZE){
		List ilist = NULL;
		int list_is_empty = 1;
		group_times += 1;
		int ridiculous = make_interaction_list (1, root, node, &ilist,&list_is_empty);
		group_sum_total_len += ridiculous;

	//	printf("ridiculous %d\n", ridiculous);
		
		//printf("%d\n", list_aggregate(ilist, yes)); 
		//sum force
		if (!list_is_empty){
			sum_interaction_list (node, ilist);
		}
		//use direct sum for the forces within a group
		direct_sum (node);
		//delete ilist
		destroyList (ilist, free);
	}else{
		for (int i = 0; i < 8; ++i){
			calculate_force (root,node->children[i]);
		}
	}	
}
