#ifndef OTREE_H
#define OTREE_H
#include "point_mass.h"
//for ease of debuggging
#define OTREE_NODE_CAP 5
//stop updating centre of mass once the
//last update moves the centre of mass
//less than one one thousands of the node's length
#define COM_RESOLUTION 1000
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
//O(logn)
otree_t* otree_insert (otree_t* tree, pmass_t* particle, int cal_com);
//called on a leaf node
//check the position of the i'th particle
//if it is out of bound, rotate it upwards
//until we find a parent node where it can be inserted again
//return the new leaf node where the particle now is
//O(logn)
otree_t* otree_relocate(otree_t* tree, int i, pmass_t* particle);

//fix the centre of mass of all nodes affected by relocating
//O(logn)
void otree_fix_com (otree_t* src, otree_t* dst, pmass_t* old_part, 
					pmass_t* new_part);
//check the tree constraints
//to make sure the functions are correct
void check_constraints (otree_t* root);
#endif
