#ifndef PMASS_H
#define PMASS_H

#include <math.h>
#ifdef USE_DOUBLE
typedef double floating_point;
#else
typedef float floating_point;
#endif

#define ABS(x) ((x) >= 0? x: (-1.0)*(x))
#define NEWTON_MANTISSA ((floating_point)6.674)


typedef  struct point{
	floating_point x,y,z;
}point_t;

typedef struct pmass{
	point_t pos;
	point_t vel;
	point_t acc;
	floating_point mass;
}pmass_t;


static inline floating_point dist_between_points_sqrd(point_t* a, point_t* b){
	floating_point x_dist = ABS(a->x - b->x);	
	floating_point y_dist = ABS(a->y - b->y);
	floating_point z_dist = ABS(a->z - b->z);
	return x_dist * x_dist + y_dist * y_dist + z_dist * z_dist;
}

static inline floating_point force_between_particles (pmass_t* a, pmass_t* b){
	floating_point force = NEWTON_MANTISSA * a->mass * b->mass;
	force /= dist_between_points_sqrd (&a->pos, &b->pos);
	return force;
}

static inline pmass_t centre_of_mass (pmass_t* a, pmass_t* b){
	floating_point mass = a->mass + b->mass;
	floating_point x = (a->pos.x * a->mass + b->pos.x * b->mass)/(mass);
	floating_point y = (a->pos.y * a->mass + b->pos.y * b->mass)/(mass);
	floating_point z = (a->pos.z * a->mass + b->pos.z * b->mass)/(mass);
	return (pmass_t){.pos = (point_t){.x = x, .y = y, .z = z}, .mass = mass};
}
#endif
