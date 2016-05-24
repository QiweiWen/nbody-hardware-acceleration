#include "hwaccl.h"
#include "octtree.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

static uint16_t elements_per_write = 200;
static char* bytebuffers [NUM_PROCESSORS];
float* buffers [NUM_PROCESSORS];
static uint16_t currindexes [NUM_PROCESSORS];
static tcb_t threads [NUM_PROCESSORS];

static const char* writing_streams[NUM_PROCESSORS][NUM_PIPELINES_PER_CPU]
= {{"cpu0_ofile0", "cpu0_ofile1"}, {"cpu1_ofile0", "cpu1_ofile1"}};

static const char* reading_streams[NUM_PROCESSORS][NUM_PIPELINES_PER_CPU]
= {{"cpu0_ifile0", "cpu0_ifile1"}, {"cpu1_ifile0", "cpu1_ifile1"}};

void open_streams (uint16_t tid, int reading, int writing){
	tcb_t* tcb  = &threads [tid];
	for (int i = 0; i < NUM_PIPELINES_PER_CPU; ++i){
		if (writing)
			tcb->streams [i] = open (writing_streams [tid][i], O_WRONLY);
		if (reading)
			tcb->reading_streams [i] = open 
				(reading_streams[tid][i], O_RDONLY);
	}	
}

void hwaccl_init (void){

	for (int i = 0; i < NUM_PROCESSORS; ++i){
		bytebuffers [i] =  malloc (sizeof (char) * RAMBUFF_SIZE);
		buffers [i] = (float*)bytebuffers [i];
		currindexes [i] = 0;
		threads [i].stream_index = 0;;	
		open_streams (i, 1, 0);
	}
}


void write_target (uint16_t tid, pmass_t* tgt){
	open_streams (tid, 0, 1);
	tcb_t* tcb = &threads [tid];
	float buff[3] = {tgt->pos.x, tgt->pos.y, tgt->pos.z};
	for (int i = 0; i < NUM_PIPELINES_PER_CPU; ++i){
		size_t len = 3 * sizeof (float);
		ssize_t written = write (tcb->streams[i],buff, len);	
		assert(written == len);
	}
}
//THIS IS A BLOCKING OPERATION
vector_t read_result (uint16_t tid){
	float floatbuf [3];
	tcb_t* tcb = &threads [tid];
	vector_t partial_force = {0,0,0};
	vector_t increment;
	ssize_t bytes_left;
	ssize_t bytes_read;
	ssize_t total_read;
	char* buffer_pointer;
	for (int i = 0; i < NUM_PIPELINES_PER_CPU; ++i){
		int fd = tcb->reading_streams [i];
		bytes_left = 3* sizeof(float);
		buffer_pointer = (char*)floatbuf;
		total_read = 0;
		while (bytes_left){
			bytes_read = read (fd, buffer_pointer,bytes_left); 
			if (bytes_read < 0) {
				assert (!"error while reading\n");
			}else{
				bytes_left -= bytes_read;
				buffer_pointer += bytes_read;
				total_read += bytes_read;
			}
		}
		if (total_read < 3* sizeof(float)){
			printf ("something went wrong, only %d bytes read\n", total_read);
			char* bytes = (char*)floatbuf;
			for (int i = 0; i < total_read; ++i){
				printf ("%02x\n", bytes[i]);
			}
			assert (!"fix the hardware");
		}
		increment = 
			(vector_t) {.x = floatbuf[0], .y = floatbuf [1], .z = floatbuf[2]};
		vector_add (&partial_force, &increment);	
	}	
	return partial_force;
}


static void add_to_buffer_custom (uint16_t tid, pmass_t* part, int force_flush){
	tcb_t* tcb = &threads [tid];	
	float* buffer = buffers[tid];
	
	if (!force_flush){
		int floatindex = currindexes[tid] * 4;		
		buffer [floatindex++] = part->pos.x;
		buffer [floatindex++] = part->pos.y;
		buffer [floatindex++] = part->pos.z;
		buffer [floatindex]   = part->mass;
		currindexes [tid]++;
	}
	if (force_flush || currindexes [tid] == elements_per_write){

		//flush to dma buffer
		int stream_to_use = tcb-> stream_index;

		int fd = tcb->streams [stream_to_use];
		tcb->stream_index = (tcb->stream_index + 1)%NUM_PIPELINES_PER_CPU;

		size_t len = currindexes[tid] * 4 * sizeof(float);
		ssize_t written = write (fd, buffer, len);
		
		assert (written == len);
		currindexes [tid] = 0;
		if (force_flush){	
			//flush from dma buffers downstream
			close_streams (tid, 0, 1);
		}
	}
}

void add_to_buffer (uint16_t tid, pmass_t* part){
	add_to_buffer_custom (tid, part, 0);
}

void flush_to_dma (uint16_t tid){
	add_to_buffer_custom (tid, NULL, 1);
}

void close_streams (uint16_t tid, int reading, int writing){
	tcb_t* tcb = &threads [tid];
	float zerobuf[4] = {0};
	for (int i = 0; i < NUM_PIPELINES_PER_CPU; ++i){
		if (writing){
			write (tcb->streams [i], zerobuf, sizeof(float) * 4);
			close (tcb->streams [i]);
		}
		if (reading) 
			close (tcb->reading_streams [i]);
	}
}

//not threadsafe; call in serial code only
void update_ilist_len (size_t newlen){
	elements_per_write = newlen / 10;
	assert (elements_per_write < RAMBUFF_SIZE/BUFFER_ENTRY_SIZE);  
	dbprintf ("NEW PARTS PER WRITE: %hu\n", elements_per_write);
}
