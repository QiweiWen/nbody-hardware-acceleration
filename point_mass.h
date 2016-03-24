#ifndef PMASS_H
#define PMASS_H

#include <math.h>
#ifdef USE_DOUBLE
typedef double float_t;
#else
typedef float float_t;
#endif

#define ABS(x) (x >= 0? x: (-1)*x)
#define NEWTON_MANTISSA ((float_t)6.674)
#define NEWTON_CONSTANT ((float_t)(NEWTON_MANTISSA * pow((double)10,-11)))

typedef  struct point{
	float_t x,y,z;
}point_t;

typedef struct pmass{
	point_t pos;
	point_t vel;
	point_t acc;
	float_t mass;
}pmass_t;


static inline float_t dist_between_points_sqrd(point_t* a, point_t* b){
	float_t x_dist = ABS(a->x - b->x);
	float_t y_dist = ABS(a->y - b->y);
	float_t z_dist = ABS(a->z - b->z);
	return x_dist * x_dist + y_dist * y_dist + z_dist * z_dist;
}

static inline float_t force_between_particles (pmass_t* a, pmass_t* b){
	float_t force = NEWTON_CONSTANT * a->mass * b->mass;
	force /= dist_between_points_sqrd (&a->pos, &b->pos);
	return force;
}

static inline pmass_t centre_of_mass (pmass_t* a, pmass_t* b){
	float_t mass = a->mass + b->mass;
	float_t x = (a->pos.x * a->mass + b->pos.x * b->mass)/(mass);
	float_t y = (a->pos.y * a->mass + b->pos.y * b->mass)/(mass);
	float_t z = (a->pos.z * a->mass + b->pos.z * b->mass)/(mass);
	return (pmass_t){.pos = (point_t){.x = x, .y = y, .z = z}, .mass = mass};
}
#endif
