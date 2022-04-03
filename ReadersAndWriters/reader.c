// Creat: 25/04/2021
// Editat:
// Acest program citeste date, de tipul intreg, care sunt stochate intr-un sir de intregi care este partajat 
// cu alte procese de tipul scriitori

#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <time.h>

#define QUEUE_SIZE 10

typedef struct _Tasks {
	int queue[QUEUE_SIZE];
	int pos_take;
	int pos_empty;
	int count;
	int isOpen;
	int generated_tasks;
	int writerCount;
	int readerCount;
}TASKS;

char FILE_NAME[] = "Tasks.txt";
char INIT_SEM_NAME[] = "init_sem";
char WRITE_SEM_NAME[] = "write_sem";
char READ_SEM_NAME[] = "read_sem";
char QUEUE_READERS_SEM_NAME[] = "queue_readers_sem";
char Z_SEM_NAME[] = "z_sem";

sem_t *init_sem;
sem_t *write_sem;
sem_t *read_sem;
sem_t *queue_readers_sem;
sem_t *z_sem;


int main(void)
{
	init_sem = sem_open(INIT_SEM_NAME, O_CREAT, 0644, 0);
	if (init_sem == SEM_FAILED)
	{
		perror("Unable to create INIT semaphore");
		sem_unlink(INIT_SEM_NAME);
		exit(2);
	}

	struct timespec waitTime;
	if (clock_gettime(CLOCK_REALTIME, &waitTime) < 0)
	{
		perror("Error get time");
		exit(5);
	}
	waitTime.tv_sec += 2;

	int sem_value;
	sem_getvalue(init_sem, &sem_value);
	printf("Reader %d wait after init_sem with value %d\n", getpid(), sem_value);
	if (sem_timedwait(init_sem, &waitTime) < 0)//asteapta ca sa se realizeze initializarea
	{
		perror("Reader timeout initialization, suspend");
		exit(6);
	}

	int fd;
	if ((fd = open(FILE_NAME, O_RDWR)) < 0)
	{
		perror("Error create file");
		exit(1);
	}

	TASKS *tasks = mmap(NULL, sizeof(TASKS), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (tasks == NULL)
	{
		perror("Unable to map TASKS struct");
		sem_close(init_sem);
		sem_unlink(INIT_SEM_NAME);
		exit(3);
	}


	write_sem = sem_open(WRITE_SEM_NAME, O_CREAT, 0644, 1);
	if (write_sem == SEM_FAILED)
	{
		perror("Unable to create WRITE semaphore");
		sem_unlink(WRITE_SEM_NAME);
		exit(2);
	} 

	read_sem = sem_open(READ_SEM_NAME, O_CREAT, 0644, 1);
	if (read_sem == SEM_FAILED)
	{
		perror("Unable to create READ semaphore");
		sem_unlink(READ_SEM_NAME);
		exit(2);
	} 

	queue_readers_sem = sem_open(QUEUE_READERS_SEM_NAME, O_CREAT, 0644, 1);
	if (queue_readers_sem == SEM_FAILED)
	{
		perror("Unable to create READ semaphore");
		sem_unlink(QUEUE_READERS_SEM_NAME);
		exit(2);
	}

	z_sem = sem_open(Z_SEM_NAME, O_CREAT, 0644, 1);
	if (z_sem == SEM_FAILED)
	{
		perror("Unable to create READ semaphore");
		sem_unlink(Z_SEM_NAME);
		exit(2);
	}

	while (1)
	{
		//printf("Reader %d wait after z_sem\n", getpid());
		sem_wait(z_sem);
		//printf("Reader %d wait after read_sem\n", getpid());
		sem_wait(read_sem);
		//printf("Reader %d wait after queue_sem\n", getpid());
		sem_wait(queue_readers_sem);
		tasks->readerCount++;
		if (tasks->readerCount == 1)
		{
			printf("Reader %d wait after write_sem\n", getpid());
			sem_wait(write_sem);
		}
		sem_post(queue_readers_sem);
		sem_post(read_sem);
		sem_post(z_sem);

		if (tasks->isOpen == 0)
			break;

		printf("Reader %d :", getpid());
		for (int i = 0; i < QUEUE_SIZE; i++)
		{
			printf("[%d]=%d ",
				(tasks->pos_empty + i) % QUEUE_SIZE,
			       	tasks->queue[(tasks->pos_empty + i) % QUEUE_SIZE]
				);
		}
		printf("\n");

		//printf("Reader %d wait after queue_sem\n", getpid());
		sem_wait(queue_readers_sem);
		tasks->readerCount--;
		if (tasks->readerCount == 0)
			sem_post(write_sem);
		sem_post(queue_readers_sem);
	}

	sem_close(init_sem);
	sem_close(write_sem);
	sem_close(read_sem);
	sem_close(queue_readers_sem);
	sem_close(z_sem);
	sem_unlink(INIT_SEM_NAME);
	sem_unlink(WRITE_SEM_NAME);
	sem_unlink(READ_SEM_NAME);
	sem_unlink(QUEUE_READERS_SEM_NAME);
	sem_unlink(Z_SEM_NAME);
	
	printf("Reader %d leaves\n", getpid());

	exit(0);
}
