#ifndef PMASS_H
#define PMASS_H
#include "config.h"
#include <math.h>

#define ABS(x) ((x) >= 0? x: (-1.0)*(x))
#define NEWTON_CONSTANT (0.00000000006674)

typedef  struct point{
	float x,y,z;
}point_t;

typedef point_t vector_t;

typedef struct pmass{
	point_t pos;
	point_t vel;
	point_t acc;
	float mass;
}pmass_t;


static inline float dist_between_points_sqrd(point_t* a, point_t* b){
	float x_dist = ABS(a->x - b->x);	
	float y_dist = ABS(a->y - b->y);
	float z_dist = ABS(a->z - b->z);
	return x_dist * x_dist + y_dist * y_dist + z_dist * z_dist;
}

static inline float acceleration_on_particle (pmass_t* a, pmass_t* b){
	float acc = NEWTON_CONSTANT * b->mass;
	acc /= dist_between_points_sqrd (&a->pos, &b->pos);
	return acc;
}

static inline pmass_t centre_of_mass (pmass_t* a, pmass_t* b){
	float mass = a->mass + b->mass;
	float x = (a->pos.x * a->mass + b->pos.x * b->mass)/(mass);
	float y = (a->pos.y * a->mass + b->pos.y * b->mass)/(mass);
	float z = (a->pos.z * a->mass + b->pos.z * b->mass)/(mass);
	return (pmass_t){.pos = (point_t){.x = x, .y = y, .z = z}, .mass = mass};
}


static inline void vector_scalar_mult (point_t* vec, float scale){
	vec->x *= scale;
	vec->y *= scale;
	vec->z *= scale;
}


static inline void vector_scalar_div (point_t* vec, float scale){
	vec->x /= scale;
	vec->y /= scale;
	vec->z /= scale;
}

static inline void vector_gravity (pmass_t* acted, pmass_t* object, point_t* res){
	//to replicate what the FPGA does as much as possible
	float dx = object->pos.x - acted->pos.x;
	float dy = object->pos.y - acted->pos.y;
	float dz = object->pos.z - acted->pos.z;
	float r2 = dx*dx + dy*dy + dz*dz;
	float r  = sqrt (r2);
	float r3 = r2 * r;
	float GM_by_r3 = NEWTON_CONSTANT * object->mass / r3;
    res->x = GM_by_r3 * dx;
	res->y = GM_by_r3 * dy;
	res->z = GM_by_r3 * dz;	
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
	dbprintf("(%.20f, %.20f, %.20f)\n", vec->x, vec->y, vec->z);
}

static void __attribute__ ((unused)) print_pmass (void* data){
	pmass_t* part = (pmass_t*)data;
	dbprintf("===\n");
	dbprintf ("mass %.20f\n", part->mass);
	dbprintf("coord: ");print_vector (&part->pos);
	dbprintf("acc: ");print_vector (&part->acc);	
	dbprintf("vel: ");print_vector (&part->vel);

}
#endif
