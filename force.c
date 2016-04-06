#include "octtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#define DELTA_T 1

//given a pointer to a otree_t, update the position and velocity of the leaves
void force_calculate_forces(otree_t* root) {
   otree_t** leaf_array[1000];
	int num_leaves = get_leaves(root, leaf_array, 0, 1000);
   update_leaves(leaf_array, num_leaves);
}

//r(t) = r0 + v0*t + 0.5*a*t*t
floating_point calculate_position_component(floating_point pos, floating_point vel, floating_point acc) {
	return pos + vel * DELTA_T + 0.5 * acc * DELTA_T * DELTA_T;
}

//v(t) = v0 + a*t
floating_point calculate_velocity_component(floating_point vel, floating_point acc) {
	return vel + acc * DELTA_T;
}

//given an array of leaves, update the position and velocity
//of each leaf based on the acceleration it has
void update_leaves(otree_t** leaves, int num_leaves) {
	//loop over the leaves 
	for (int i = 0; i < num_leaves; i++) {
		//loop over the particles
		for (int j = 0; j < leaves[i]->num_particles; j++) {
         pmass_t* particle = &leaves[i]->particles[j];
			point_t* a = &particle->acc;
			
			//save the initial position and velocity
			point_t* pos_init = &particle->pos;
			point_t* vel_init = &particle->vel;
			
			//update the position of the particle
			particle->pos.x = calculate_position_component(pos_init->x, vel_init->x, a->x);
			particle->pos.y = calculate_position_component(pos_init->y, vel_init->y, a->y);
			particle->pos.z = calculate_position_component(pos_init->z, vel_init->z, a->z);
			
			//update the velocity of the particle
			particle->vel.x = calculate_velocity_component(vel_init->x, a->x);
			particle->vel.y = calculate_velocity_component(vel_init->y, a->y);
			particle->vel.z = calculate_velocity_component(vel_init->z, a->z);
		}
		
	}
}
