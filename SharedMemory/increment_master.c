// Autor: Sopterean Mihai
// Creat: 22/05/2021
// Editat:
// Se cere a incrementa strict alternativ un contor de N procese neinrudite, folosind memoria partajata
// Acesta este procesul 1, care creaza si initializeaza memoria partajata
// Primeste ca argument nr N, care reprezinta cate procese vor incrementa contorul
// inclusiv procesul 1

#include "increment.h"

int N;
SHARED_MEM *pshm;

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage %s N\n", argv[0]);
		exit(1);
	}
	
	int total;
	if (sscanf(argv[1], "%d", &total) < 1)
	{
		fprintf(stderr, "Invalid N\n");
		exit(2);
	}

	int fd = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, 0600);
	if (fd < 0)
	{
		perror("Error to create shared memory object");
		exit(3);
	}
	
	ftruncate(fd, sizeof(SHARED_MEM));
	pshm = mmap(NULL, sizeof(SHARED_MEM), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (pshm == (void*)-1)
	{
		perror("Error to map shader memory");
		close(fd);
		shm_unlink(SHARED_MEM_NAME);
		exit(4);
	}
	close(fd);

	sem_init(&pshm->mutex, 1, 1);
	pshm->N = total;
	pshm->counter = 0;
	
	N = 0;

	while (1)
	{
		sem_wait(&pshm->mutex);

		if (pshm->counter >= 100)
		{
			sem_post(&pshm->mutex);
			break;
		}

		if (pshm->counter % pshm->N == N)
		{
			pshm->counter++;
			printf("Process %d incremented counter to %d\n", N, pshm->counter);
		}

		sem_post(&pshm->mutex);
	}

	//sem_destroy(&pshm->mutex);
	shm_unlink(SHARED_MEM_NAME);
	
	return 0;
}
