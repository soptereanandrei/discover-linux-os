// Autor: Sopterean Mihai
// Creat: 23/05/2021
// Editat:
// Programul citeste dintr-un buffer partajat prin intermediul memorie partajate 
// ce un alt process a scris si afiseaza in stdout continutul bufferului

#include "writer_reader.h"
#include <semaphore.h>
#include <sys/mman.h>

int main(void)
{
	int fd = shm_open(CONTROL_STRUCT_NAME, O_RDWR, 0);
	if (fd < 0)
	{
		perror("Error to open shared memory for control struct");
		exit(3);
	}
	CONTROL *control = mmap(NULL, sizeof(CONTROL), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);

	char *buffer = (char*)malloc(sizeof(char) * BUFFER_LEN);
	fd = shm_open(BUFFER_NAME, O_RDWR, 0);
	if (fd < 0)
	{
		perror("Error to open shared memory for buffer");
		exit(4);
	}
	char *sharedBuff = mmap(buffer, sizeof(char) * BUFFER_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (sharedBuff != buffer)
	{
		fprintf(stderr, "Buffer was allocated at another address\n");
	}
	close(fd);

	while (1)
	{
		sem_wait(&control->mutexWriter);
		//printf("Reader entered in critical section\n");

		if (control->lineLength == 0)
		{
			sem_post(&control->mutexReader);
			printf("Writer reach end of file\n");
			break;
		}

		write(1, sharedBuff, control->lineLength);

		sem_post(&control->mutexReader);
	}

	sem_destroy(&control->mutexReader);
	sem_destroy(&control->mutexWriter);

	shm_unlink(CONTROL_STRUCT_NAME);
	shm_unlink(BUFFER_NAME);

	printf("Writer end\n");

	return 0;
}
