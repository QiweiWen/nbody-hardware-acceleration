#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main (int argc, char** argv){
	assert (argc == 5);
	int min_nodesize = atoi(argv[1]);
	int max_nodesize = atoi(argv[2]);
	int increment    = atoi(argv[3]);
	int size =		   atoi(argv[4]);
	char stackbuf[500];
	char stackbuf2[500];
	system ("rm -rf result");
	system ("mkdir result");
	for (int num = min_nodesize; num <= max_nodesize; num += increment){
		sprintf (stackbuf, "sed -i s/\"^#define OTREE_NODE_CAP.*\"/\"#define OTREE_NODE_CAP %d\"/ config.h", num);
		system (stackbuf);
		system ("make clean && make");
		system ("./nbody -f input -d 100 -l");
		sprintf (stackbuf, "%dg%d.gprofout", size, num * 3);
		sprintf (stackbuf2, "gprof nbody gmon.out > result/%s",stackbuf);
		system (stackbuf2);
	}
	return 0;
}
