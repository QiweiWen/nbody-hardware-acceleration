#ifndef CONFIG_H
#define CONFIG_H
#include <stdio.h>

#define NUM_PROCESSORS 1
#define HWACCL

//#define DEBUG
#define ANIM
#define TIMESTEP "0y10d0s"

#define OTREE_NODE_CAP 60
#define CYCLES_PER_GARBAGE_COLLECT 10
#define CYCLES_PER_WRITE 1

//maximum size of a node (not neccessarily leaf)
//for all particles under it to share the 
//same interactio list
#define GROUP_SIZE (OTREE_NODE_CAP * 3) 
#if (GROUP_SIZE < OTREE_NODE_CAP)
	#error "GROUP SIZE must be larger than leaf node size"
#endif
#define BH_THETA 0.4
#define MIN_MASS 0.5


#ifdef DEBUG
#define dbprintf(...) printf(__VA_ARGS__)
#else
#define dbprintf(...)
#endif
#endif

