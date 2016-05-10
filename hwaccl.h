#ifndef HWACCL_H
#define HWACCL_H
#include <stdint.h>
#include <pthread.h>
#include "point_mass.h"
#define NUM_PROCESSORS 2
//this is a buffer in cacheable ram which will hold a number of
//interaction list elements before dumping it to the devfile
//1MB is much more than enough but WTH
#define RAMBUFF_SIZE (1<<20)
#define NUM_PIPELINES_PER_CPU 2
#define ENTRY_SIZE 16 
#define CACHELINESIZE 64
//for a CPU to have potentially more pipeline units than one
//write elements of an interaction list into all of them
//round robin
typedef struct {
	pthread_t thread;	
	int streams [NUM_PIPELINES_PER_CPU];
	int stream_index;
	int reading_streams [NUM_PIPELINES_PER_CPU];
}tcb_t;
//how many elements the rambuffer is to hold before writing to the devfile
//large number = fewer syscalls, less parallelism
//small number = more syscalls overhead, more parallelism

void hwaccl_init (uint16_t elements_per_write);
uint16_t get_tid (void);

void write_target (uint16_t tid, pmass_t* tgt);
vector_t read_result (uint16_t tid);
void add_to_buffer (uint16_t tid, pmass_t* part);
void flush_to_dma (uint16_t tid);
void close_streams (uint16_t tid);
#endif
