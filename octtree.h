#ifndef OTREE_H
#define OTREE_H
#include "point_mass.h"
#define OTREE_NODE_CAP 50
typedef struct otree{
	pmass_t centre_of_mass;
	float_t side_len;
	//the corner with the lowest values of x y and z
	point_t corner;
	int num_particles;
	//particles in this node and its children
	int total_particles;
	pmass_t particles[OTREE_NODE_CAP];
	struct otree* children[8];
	struct otree* parent;
}otree_t;

otree_t* otree_new(float_t side_len);
void otree_free (otree_t* tree);
otree_t* otree_insert (otree_t* tree, pmass_t* particle);
//we don't delete from the tree
//collisionless, no black holes either

#endif
