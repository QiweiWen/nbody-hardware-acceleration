
#include "octtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include "force_calc.h"
#include <string.h>
#include <ctype.h>
#include "simulation.h"

int get_leaves(otree_t* tree,  otree_t** res, 
		       int curr_ind, int wanted){
	
	int tmp = 0;
	int accu = 0;
	if (tree->children[0] == NULL){
		if (tree->num_particles > 0){
			res[curr_ind] = tree;
			return 1;
		}else return 0;
	}
	for (int i = 0; i < 8; ++i){
		if (curr_ind == wanted){
			return accu; 
		}
		tmp = get_leaves (tree->children[i],res,curr_ind,wanted);
		curr_ind += tmp;
		accu += tmp;
	}
	return accu;
}

void test_inline_func(void){
	pmass_t a = {.pos =  {.x = 5.241, .y  = 9.211, .z = 1.23}};
	a.mass = 234.1;


	pmass_t b = {.pos = {.x = 15.231, .y  = 91.31, .z = 11.3}};
	b.mass = 21.34;

	point_t force;
	vector_gravity (&a, &b, &force);

	printf("(%.16lf, %.16lf, %.16lf)\n", force.x, force.y, force.z);
}


#define printerr(...) fprintf(stderr, __VA_ARGS__)

static void print_help (void){
	printerr ("Usage: \n"
			  "-y: years \n"
			  "-d: days \n"
			  "-s: seconds \n"
			  "--anim: produce animation file \n");	  
}


int main (int argc, char** argv){
	int animflag = 0;
	int animindex = -1;
	for (int i = 0; i < argc; ++i){
	
		if (!strcmp (argv[i],"--anim")) {
			animflag = 1;
			animindex = i;
		}
	}
	if (animindex != -1){
		
		for (int i = animindex; i < argc - 1; ++i){
			argv[i] = argv[i + 1];
		}
		argc--;
	}

	char* secondstring = "0";
	char* daystring = "0";
	char* yearstring = "0";
	int timearg_count = 0;
	
	char* filename;
	int fileflag = 0;

	char c;
	while ((c = getopt (argc, argv, "y:d:s:f:")) != -1)
		switch (c)
		{
			case 's':
				++timearg_count;
				secondstring = optarg;
			break;
			case 'd':
				++timearg_count;
				daystring = optarg;
			break;
			case 'y':
				++timearg_count;	
				yearstring = optarg;
			break;
			case 'f':
				fileflag = 1;
				filename = optarg;
			break;
			case 'h':
				print_help();
			break;
			case '?':
				if (optopt == 's' || optopt == 'y' || optopt == 'd' || optopt == 'f')
					fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf (stderr,
					         "Unknown option character `\\x%x'.\n",
							optopt);
			return 1;
		  default:
			abort ();
		}
	if (timearg_count == 0){
		print_help();
		exit(1);	
	}
	if (fileflag == 0){
		print_help();
		exit(1);
	}
	FILE* infile = fopen (filename, "r");
	if (!infile){
		printf("Failed to open file %s\n", filename);
		exit (1);
	}
	if (animflag){
#ifndef ANIM
		#error "the --anim flag needs the optioned \"ANIM\" enabled in config.h"
#endif
	}
	//re-format the input time
	int years = atoi (yearstring), days = atoi (daystring), seconds = atoi (secondstring);
	days += (seconds / SECS_IN_DAY);
	seconds = seconds % SECS_IN_DAY;

	years += (days / DAYS_IN_YEAR);	
	days = days % DAYS_IN_YEAR;
	
	simulation (years, days, seconds, infile, animflag);	

	return 0;
}
