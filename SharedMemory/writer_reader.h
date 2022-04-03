#ifndef P3
#define P3

#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_LEN 100

typedef struct _CONTROL {
	sem_t mutexReader;
	sem_t mutexWriter;
	int lineLength;
}CONTROL;

const char CONTROL_STRUCT_NAME[] = "/control_struct";
const char BUFFER_NAME[] = "/buffer";

#endif
