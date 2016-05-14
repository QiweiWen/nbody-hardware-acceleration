#include "octtree.h"
#include "point_mass.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "list.h"
#include <stdint.h>
#include "hwaccl.h"

#ifdef HWACCL
#include "hwaccl.h"
#endif
#include <semaphore.h>
#include <sys/sem.h>
#include <pthread.h>


#define IS_PARTICLE 1
#define IS_COM      2
#if NUM_PROCESSORS > 1

extern pthread_t ilist_threads [NUM_PROCESSORS];
//thread waits on "control" to start execution
//main thread waits on "result" for end of computation
extern sem_t     ilist_thread_control[NUM_PROCESSORS];
extern sem_t     ilist_thread_result [NUM_PROCESSORS];

#endif

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
		for (dlnode_t* curr = currnode->particles->first;
			curr != NULL; curr = curr->next)
		{
			if (*list_is_empty){
				add_list (curr->key, *ilist, IS_PARTICLE); 
				*list_is_empty = 0;
			}else{
				*ilist = list_push (*ilist, curr->key, IS_PARTICLE);
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
		
			if (*list_is_empty){
				add_list (&currnode->centre_of_mass, *ilist, IS_COM); 
				*list_is_empty = 0;
			}else{
				*ilist = list_push (*ilist, &currnode->centre_of_mass, IS_COM);
			}	
			++res;
		}	
	}
	return res;
}

static void __attribute__ ((noinline)) sum_interaction_list (otree_t* node, List ilist)  {

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
	//print_vector (&node->centre_of_mass.acc);
}

static void direct_sum_force (otree_t* root, otree_t* currnode, List ilist){
	if (currnode->children[0] == NULL){
		//apply effect of ilist on every particle
		for (dlnode_t* curr = currnode->particles->first;
					   curr != NULL; curr = curr->next)
		{
			List head = ilist;
			pmass_t* part = (pmass_t*)curr->key;
			point_t force;
			part->acc.x = 0;
			part->acc.y = 0;
			part->acc.z = 0;
			while (head != NULL){
				part = (pmass_t*)head->key;
				
				if (vector_equal ( & ((pmass_t*)curr->key)->pos, &part->pos) ){
					head = head->next;
					continue;
				}
				vector_gravity ((pmass_t*)curr->key, part, &force);
				vector_add (& ((pmass_t*)curr->key)->acc, &force);
				head = head->next;
			}
			vector_add (& ((pmass_t*)curr->key)->acc, &root->centre_of_mass.acc);	
		//	print_vector (& ((pmass_t*)curr->key)->acc);
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
	if (!list_is_empty){
		direct_sum_force (node, node, ilist);
	}
	destroyList (ilist, NULL);
}

void calculate_force (otree_t* root,otree_t* node){
	assert(node);

	if (node->total_particles < GROUP_SIZE){
		List ilist = NULL;
		int list_is_empty = 1;
		
		 make_interaction_list (1, root, node, &ilist,&list_is_empty);		
		//sum force
		if (!list_is_empty){
			sum_interaction_list (node, ilist);
		}
		//use direct sum for the forces within a group
		direct_sum (node);
		//delete ilist
		destroyList (ilist, NULL);
	}else{
		for (int i = 0; i < 8; ++i){
			calculate_force (root,node->children[i]);
		}
	}	
}

#ifdef HWACCL
void hwaccl_make_ilist (uint16_t tid, otree_t* currnode, otree_t* target);
void hwaccl_calculate_force (uint16_t tid, otree_t* root, otree_t* node){
	assert (node);
	if (node->total_particles < GROUP_SIZE){
		write_tgt (tid, &node->centre_of_mass);		
		hwaccl_make_ilist (tid, root, node);
		flush_to_dma (tid);
		vector_t force = read_result (tid);
		node->centre_of_mass.acc = force;
		direct_sum (node);
	}else{
		for (int i = 0; i < 8; ++i){
			hwaccl_calculate_force (tid, root, node->children[i]);
		}
	}
}
#endif

#if NUM_PROCESSORS > 1
void multithread_calculate_force (uint16_t tid, otree_t* root){
	int num = 8 / NUM_PROCESSORS;	
	int start = tid* num;
	for (int i = 0; i < num; ++i){
#ifdef HWACCL
		hwaccl_calculate_force (tid, root, root->children [i + start]);
#else
		calculate_force (root, root->children[i + start]);
#endif
	}
}
#endif
