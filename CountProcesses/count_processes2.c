// Autor: Sopterean Mihai
// Creat: 25/04/2021
// Editat:
// Programul contorizeaza cate procese se creeaza

#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>

char FILE_NAME[] = "contor.txt";
char SEM_NAME[] = "sem";

int main()
{
	pid_t root = getpid();
	
	int fd;
	if ((fd = open(FILE_NAME, O_CREAT | O_RDWR, 0644)) < 0)
	{
		perror("Error open file");
		exit(1);
	}
	int zero = 0;
	write(fd, &zero, sizeof(int));
	int *contor = mmap(NULL, sizeof(int), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	close(fd);

	sem_t *mutex = sem_open(SEM_NAME, O_CREAT, 0644, 1);
	if (mutex == SEM_FAILED)
	{
		perror("Unable to create semaphore");
		exit(2);
	}

	for (int i = 1; i <= 10; i++)
	{
		if (fork() < 0)
		{
			perror("fork error");
			exit(2);
		}
	}
	sem_wait(mutex);
	*contor = *contor + 1;
	sem_post(mutex);

	while (wait(NULL) > 0); //wait after all children before exit

	if (getpid() == root)
	{
		printf("%d \n", *contor);
		sem_close(mutex);
		sem_unlink(SEM_NAME);
	}

	exit(0);
}
