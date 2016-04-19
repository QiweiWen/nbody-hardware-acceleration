#include "point_mass.h"
#include "octtree.h"
#include "force_calc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>


extern uint64_t direct_sum_times;
extern uint64_t direct_sum_total_len;

extern uint64_t group_times;
extern uint64_t group_sum_total_len;

extern uint64_t sum_ilist_count;

static void print_vector (point_t* vec){
	dbprintf("(%.20lf, %.20lf, %.20lf)\n", vec->x, vec->y, vec->z);
}

static void print_pmass (void* data){
	pmass_t* part = (pmass_t*)data;
	dbprintf("===\n");
	dbprintf ("mass %.20lf\n", part->mass);
	dbprintf("coord: ");print_vector (&part->pos);
	dbprintf("acc: ");print_vector (&part->acc);	
	dbprintf("vel: ");print_vector (&part->vel);

}

static inline int cmp_int (int a, int b){
	if (a < b) return -1;
	if (a > b) return 1;
	return 0;
}

static int done (int tgt_yr, int tgt_d, int  tgt_s, 
						int curr_yr, int curr_d, int curr_sec)
{
	//lexicographic comparison
	int yr_result = cmp_int (tgt_yr, curr_yr),
		d_result  = cmp_int (tgt_d, curr_d),
		s_result  = cmp_int (tgt_s, curr_sec);
	if (yr_result == -1) return 1;
	if (yr_result == 1) return 0;
	if (d_result == -1) return 1;
	if (d_result == 1) return 0;
	if (s_result == 1) return 0;
	return 1;
}

#define DAYS_IN_YEAR 365
#define SECS_IN_DAY  86400


static inline void do_integration (pmass_t* particle, uint64_t total_seconds){
	point_t added = particle->acc;
	point_t old_vel = particle->vel;
	//scale the acceleration to add
	added . x *= total_seconds;
	added . y *= total_seconds;
	added . z *= total_seconds;

	vector_add (&particle->vel, &added);
	
	added.x = (particle->vel.x + old_vel.x)/2;	
	added.y = (particle->vel.y + old_vel.y)/2;
	added.z = (particle->vel.z + old_vel.z)/2;
	
	added . x *= total_seconds;
	added . y *= total_seconds;
	added . z *= total_seconds;
	
	vector_add (&particle->pos, &added);
}

//need to worry about units now.
//bloody good fun
//mass in kg, distance in metres, velocity in m/s
//force in Newtons
static void integrate (otree_t* node, int years, int days, int seconds, int dump, FILE* ofile){
	pmass_t* the_particle;
	uint64_t total_seconds = (uint64_t)days * SECS_IN_DAY +
							 (uint64_t)years * SECS_IN_DAY * DAYS_IN_YEAR +
							 (uint64_t)seconds;

	if (node -> children[0] == NULL){
		/*
		for (int i = 0; i < node->num_particles; ++i){
			the_particle = &(node->particles[i]);
			do_integration (the_particle, total_seconds);
			if (dump){
				assert (ofile);
				fprintf (ofile, "(%.16lf, %.16lf, %.16lf)\n", the_particle->pos.x, the_particle->pos.y, the_particle->pos.z);
			}
			if (node != otree_relocate(node,i,NULL)){
				//so the node just moved elsewhere	
				i--;
			}
		}
	*/
		dlnode_t* curr = node->particles->first,
				* last = curr;
		pmass_t old, new;
		while (curr != NULL){
			the_particle = (pmass_t*)curr->key;
			last = curr;
			old = *the_particle;
			do_integration (the_particle, total_seconds);
			new = *the_particle;

			if (dump){
				assert (ofile);
				fprintf (ofile, "(%.16lf, %.16lf, %.16lf)\n", the_particle->pos.x, the_particle->pos.y, the_particle->pos.z);
			}
			otree_t* new_leaf = otree_relocate (node, curr);
			otree_fix_com (node, new_leaf, &old, &new);
			if (new_leaf != node){
				curr = last;
			}
			curr = curr->next;
		}
	}else{
		for (int i = 0; i < 8; ++i){
			integrate (node->children[i], years, days, seconds, dump, ofile);
		}
	}
}

static void run_simulation (int years, int days, int seconds, otree_t* root, int anim){
	int ts_years, ts_days, ts_secs;
	sscanf (TIMESTEP, "%dy%dd%ds", &ts_years, &ts_days, &ts_secs);
	assert (ts_secs < SECS_IN_DAY && ts_days < DAYS_IN_YEAR);

	FILE* ofile = NULL;
#ifdef ANIM
	if (anim){
		ofile = fopen ("simfile", "w");
		fprintf (ofile, "%lf\n", root->side_len);
	}	
#endif
	int curr_years = 0, curr_days = 0, curr_secs = 0;
	int cycles = 0;
	while (!done(years, days, seconds, curr_years, curr_days, curr_secs)){
		calculate_force (root, root);
#ifdef ANIM	
		if (anim && !(cycles % CYCLES_PER_WRITE)){
			fprintf(ofile, "====\n");
			fprintf(ofile, "time %dy%dd%ds\n", curr_years, curr_days, curr_secs);
			integrate (root, ts_years, ts_days, ts_secs, 1, ofile);
		}else
#endif
		integrate (root, ts_years, ts_days, ts_secs, 0, NULL);
		//add the time step to the current time	
		curr_secs += ts_secs;
		if (curr_secs >= SECS_IN_DAY){
			curr_secs -= SECS_IN_DAY;
			curr_days += 1;
		}
		curr_days += ts_days;
		if (curr_days >= DAYS_IN_YEAR){	
			curr_years += 1;
			curr_days -= DAYS_IN_YEAR;
		}
		curr_years += ts_years;
		if (!(cycles % CYCLES_PER_GARBAGE_COLLECT)){
			otree_garbage_collect (root);
		}
		//printf("%d years, %d days and %d secs have passed\n",curr_years,curr_days,curr_secs);
		++cycles;
	}	
}

void simulation (int years, int days, int seconds,FILE* infile, int anim){

	//get the input
	char* heapbuf = malloc(sizeof(char) * 200);
	size_t len = 200;
	int charread = 0;
	int linenum = 0;


	floating_point universe_size = 0;
	otree_t* the_tree;
	pmass_t particle;
	memset (&particle,0, sizeof (pmass_t)); 
	while ((charread = getline (&heapbuf, &len, infile)) != -1){
		if (linenum == 0){
			sscanf (heapbuf,FFMT,&universe_size);
			the_tree = otree_new (universe_size);
			dbprintf("universe size %lf\n", universe_size);
		}else{
			//the format string for the float type
			//doesnt matter for output, but does for input
			sscanf (heapbuf, "("FFMT"," FFMT"," FFMT"," FFMT")",
					&particle.pos.x, &particle.pos.y,
					&particle.pos.z, &particle.mass);	
			print_pmass (&particle);	
			otree_insert (the_tree,NULL, &particle, 1);	
		}
		++linenum;
	}
	free (heapbuf);
	run_simulation (years, days, seconds, the_tree, anim);
	printf("direct sum: %llu, %llu\ngroup: %llu, %llu\nsum_ilst: %llu\n",
			direct_sum_total_len, direct_sum_times, group_sum_total_len, group_times, sum_ilist_count);
}

