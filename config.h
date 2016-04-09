#ifndef CONFIG_H
#define CONFIG_H
#define USE_DOUBLE
#define DEBUG
#define TIMESTEP "0y0d1s"
//for ease of debuggging
#define OTREE_NODE_CAP 5
//when an update changes the COM
//by less than one one thousands
//stop updating
#define COM_RESOLUTION 1000

#ifdef DEBUG
#define dbprintf(...) printf(__VA_ARGS__)
#else
#define dbprintf(...)
#endif
#endif

