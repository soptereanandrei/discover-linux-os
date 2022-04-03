// Autor: Sopterean Mihai
// Creat: 23/05/2021
// Editat:
// Programul citeste din stdin o linie pe care o scrie intr-un buffer partajat prin 
// memorie partajata, un alt process o va citi si o va afisa pe stdout
// Programul primeste ca argument numarul de linii pe care le va citi


#include "writer_reader.h"
#include <fcntl.h>
#include <sys/mman.h>

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage %s N\n", argv[0]);
		exit(1);
	}

	int N;
	if (sscanf(argv[1], "%d", &N) < 1)
	{
		fprintf(stderr, "Invalid N\n");
		exit(2);
	}

	int fd = shm_open(CONTROL_STRUCT_NAME, O_CREAT | O_RDWR, 0600);
	if (fd < 0)
	{
		perror("Error to create shared memory for control struct");
		exit(3);
	}
	ftruncate(fd, sizeof(CONTROL));
	CONTROL *control = mmap(NULL, sizeof(CONTROL), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);

	sem_init(&control->mutexReader, 1, 1);
	sem_init(&control->mutexWriter, 1, 0);

	char *buffer = (char*)malloc(sizeof(char) * BUFFER_LEN);
	fd = shm_open(BUFFER_NAME, O_CREAT | O_RDWR, 0600);
	if (fd < 0)
	{
		perror("Error to create shared memory for buffer");
		exit(4);
	}
	ftruncate(fd, sizeof(char) * BUFFER_LEN);
	char *sharedBuff = (char*)mmap(buffer, sizeof(char) * BUFFER_LEN, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (sharedBuff != buffer)
	{
		fprintf(stderr, "Buffer was allocated at another address\n");
	}
	close(fd);

	for (int i = 0; i < N; i++)
	{
		sem_wait(&control->mutexReader);
		
		int ret;
		if ((ret = read(0, sharedBuff, BUFFER_LEN)) < 0)
		{
			perror("Error to read");
			break;
		}
		control->lineLength = ret;

		sem_post(&control->mutexWriter);
	}
	sem_wait(&control->mutexReader);
	control->lineLength = 0;//marcheaza ca s-a sfarsit citirea
	sem_post(&control->mutexWriter);
	
	shm_unlink(CONTROL_STRUCT_NAME);
	shm_unlink(BUFFER_NAME);

	printf("Reader end\n");

	return 0;
}
