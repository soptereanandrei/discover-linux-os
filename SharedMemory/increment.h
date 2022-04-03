#ifndef PB2
#define PB2

#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct _sharedmem {
	sem_t mutex;
	int N;
	int counter;
}SHARED_MEM;

const char SHARED_MEM_NAME[] = "shared_mem";

#endif
