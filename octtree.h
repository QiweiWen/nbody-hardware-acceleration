#ifndef OTREE_H
#define OTREE_H
#include "point_mass.h"
//for ease of debuggging
#define OTREE_NODE_CAP 5
typedef struct otree{
	pmass_t centre_of_mass;
	floating_point side_len;
	//the corner with the lowest values of x y and z
	point_t corner;
	int num_particles;
	//particles in this node and its children
	int total_particles;
	pmass_t particles[OTREE_NODE_CAP];
	struct otree* children[8];
	struct otree* parent;
}otree_t;

//create an empty root node
otree_t* otree_new(floating_point side_len);
//free up the memory taken by the tree
void otree_free (otree_t* tree);
//insert a node
otree_t* otree_insert (otree_t* tree, pmass_t* particle);
//called on a leaf node
//check the position of the i'th particle
//if it is out of bound, rotate it upwards
//until we find a parent node where it can be inserted again
void otree_relocate(otree_t* tree, int i, pmass_t* particle);
#endif
