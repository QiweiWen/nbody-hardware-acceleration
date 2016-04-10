#ifndef CONFIG_H
#define CONFIG_H

#define USE_DOUBLE
//#define DEBUG
#define ANIM
#define TIMESTEP "0y0d60s"
//for ease of debuggging
#define OTREE_NODE_CAP 10
//when an update changes the COM
//by less than one one thousands
//stop updating
#define COM_RESOLUTION 1000
#define CYCLES_PER_GARBAGE_COLLECT 1000
#define CYCLES_PER_WRITE 100

//maximum size of a node (not neccessarily leaf)
//for all particles under it to share the 
//same interactio list
#define GROUP_SIZE (OTREE_NODE_CAP * 5) 
#define BH_THETA 0.2

#ifdef DEBUG
#define dbprintf(...) printf(__VA_ARGS__)
#else
#define dbprintf(...)
#endif
#endif

