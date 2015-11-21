#ifndef SH_MEM_H
#define SH_MEM_H

#include <time.h>

#define MAX_FIFO_NAME_SIZE 256

typedef struct{
	int number;
	time_t openingTime;
	int duration;
	char fifoName[256];
	int servicing;
	int serviced;
	float averageTime;
}sh_mem_entry;



#endif