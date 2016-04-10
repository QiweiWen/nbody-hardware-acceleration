#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>

int main (int argc, char** argv){
	assert (argc == 5);
	double world_size = atof(argv[1]);
	int num_particles = atoi(argv[2]);
	int	max_mass = atoi(argv[3]);
	int	min_mass = atoi(argv[4]);
	srand (time (NULL));
	printf ("%lf\n", world_size);
	double x, y, z, mass;
	for (int i = 0 ; i < num_particles; ++i){	
		x = world_size * (double)rand()/(double)RAND_MAX;
		y = world_size * (double)rand()/(double)RAND_MAX;
		z = world_size * (double)rand()/(double)RAND_MAX;
		mass = min_mass + (max_mass - min_mass) * (double)rand()/(double)RAND_MAX;
		printf ("(%lf, %lf, %lf, %lf)\n",x,y,z,mass);
	}
	return 0;
}
