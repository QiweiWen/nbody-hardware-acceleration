#ifndef PMASS_H
#define PMASS_H
#include "config.h"
#include <math.h>
#ifdef USE_DOUBLE
typedef double floating_point;
#define FFMT  "%lf"
#else
typedef float floating_point;
#define FFMT  "%f"
#endif

#define ABS(x) ((x) >= 0? x: (-1.0)*(x))
#define NEWTON_CONSTANT (0.00000000006674)

typedef  struct point{
	floating_point x,y,z;
}point_t;

typedef point_t vector_t;

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

static inline floating_point acceleration_on_particle (pmass_t* a, pmass_t* b){
	floating_point acc = NEWTON_CONSTANT * b->mass;
	acc /= dist_between_points_sqrd (&a->pos, &b->pos);
	return acc;
}

static inline pmass_t centre_of_mass (pmass_t* a, pmass_t* b){
	floating_point mass = a->mass + b->mass;
	floating_point x = (a->pos.x * a->mass + b->pos.x * b->mass)/(mass);
	floating_point y = (a->pos.y * a->mass + b->pos.y * b->mass)/(mass);
	floating_point z = (a->pos.z * a->mass + b->pos.z * b->mass)/(mass);
	return (pmass_t){.pos = (point_t){.x = x, .y = y, .z = z}, .mass = mass};
}


static inline void vector_scalar_mult (point_t* vec, floating_point scale){
	vec->x *= scale;
	vec->y *= scale;
	vec->z *= scale;
}


static inline void vector_scalar_div (point_t* vec, floating_point scale){
	vec->x /= scale;
	vec->y /= scale;
	vec->z /= scale;
}

static inline void vector_gravity (pmass_t* acted, pmass_t* object, point_t* res){
	res->x = object->pos.x - acted->pos.x,
	res->y = object->pos.y - acted->pos.y,
	res->z = object->pos.z - acted->pos.z;

	floating_point scalar_acceleration = acceleration_on_particle (acted, object);
	floating_point mag = sqrt (res->x * res->x + res->y * res->y + res->z * res->z);
	
	vector_scalar_div (res, mag);
	vector_scalar_mult(res,scalar_acceleration);
}

static inline void vector_add (point_t* res, point_t* other){
	res->x += other->x;
	res->y += other->y;
	res->z += other->z;
}

static inline int vector_equal (point_t* a, point_t* b){
	return (a->x == b->x && a->y == b->y && a->z == b->z);
}

static void __attribute__ ((unused)) print_vector (point_t* vec){
	dbprintf("(%.20lf, %.20lf, %.20lf)\n", vec->x, vec->y, vec->z);
}

static void __attribute__ ((unused)) print_pmass (void* data){
	pmass_t* part = (pmass_t*)data;
	dbprintf("===\n");
	dbprintf ("mass %.20lf\n", part->mass);
	dbprintf("coord: ");print_vector (&part->pos);
	dbprintf("acc: ");print_vector (&part->acc);	
	dbprintf("vel: ");print_vector (&part->vel);

}
#endif
