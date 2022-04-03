// Autor: Sopterean Mihai
// Creat: 22/05/2021
// Editat:
// Se cere a incrementa strict alternativ un contor de N procese neinrudite, folosind memoria partajata
// Acesta este unul dintre procesele (2, N), primeste ca argument index-ul reprezentativ
// procesului actual, numarul N

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

	if (sscanf(argv[1], "%d", &N) < 0)
	{
		fprintf(stderr, "Invalid N\n");
		exit(2);
	}

	int fd = shm_open(SHARED_MEM_NAME, O_RDWR, 0);
	if (fd < 0)
	{
		perror("Error to open shared memory object");
		exit(3);
	}

	pshm = mmap(NULL, sizeof(SHARED_MEM), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (pshm == (void*)-1)
	{
		perror("Error to map shared memory");
		close(fd);
		shm_unlink(SHARED_MEM_NAME);
		exit(4);
	}
	close(fd);

	if (N < 2 || N > pshm->N)
	{
		fprintf(stderr, "N must be in interval [2, %d]\n", pshm->N);
		shm_unlink(SHARED_MEM_NAME);
		exit(5);
	}

	N = N - 1;

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

	shm_unlink(SHARED_MEM_NAME);

	return 0;
}
