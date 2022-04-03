// Autor: Sopterean Mihai
// Creat: 25/04/2021
// Editat:
// Acest program genereaza date, de tipul intreg, pe care le stocheaza intr-un sir de intregi care este partajat cu alte procese de tipul
// cititori

#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>

#define NUMBER_OF_TASKS 20
#define MAX_WRITERS 2
#define MAX_READERS 5

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
char QUEUE_WRITERS_SEM_NAME[] = "queue_writers_sem";

sem_t *init_sem;
sem_t *write_sem;
sem_t *read_sem;
sem_t *queue_writers_sem;

int main(void)
{
	int fd;
	if ((fd = open(FILE_NAME, O_CREAT | O_TRUNC | O_RDWR, 0644)) < 0)
	{
		perror("Error create file");
		exit(1);
	}
	TASKS t;
	t.pos_empty = 0;
	t.pos_take = 0;
	t.count = 0;
	t.isOpen = 1;
	t.generated_tasks = 0; 
	t.writerCount = 0;
	t.readerCount = 0;
	write(fd, &t, sizeof(TASKS));

	init_sem = sem_open(INIT_SEM_NAME, O_CREAT, 0644, 0);
	if (init_sem == SEM_FAILED)
	{
		perror("Unable to create INIT semaphore");
		sem_unlink(INIT_SEM_NAME);
		exit(2);
	}

	TASKS *tasks = mmap(NULL, sizeof(TASKS), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (tasks == NULL)
	{
		perror("Unable to map TASKS struct");
		sem_close(init_sem);
		sem_unlink(INIT_SEM_NAME);
		exit(3);
	}

	for (int i = 0; i < MAX_READERS; i++)
		sem_post(init_sem);//specifica ca initializarea a avut loc

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
		perror("Unable to create WRITE semaphore");
		sem_unlink(READ_SEM_NAME);
		exit(2);
	}

	queue_writers_sem = sem_open(QUEUE_WRITERS_SEM_NAME, O_CREAT, 0644, 1);
	if (queue_writers_sem == SEM_FAILED)
	{
		perror("Unable to create WRITE semaphore");
		sem_unlink(QUEUE_WRITERS_SEM_NAME);
		exit(2);
	}


	for (int i = 0; i < MAX_WRITERS; i++)
	{
		pid_t child = fork();
		if (child == 0)//child process va fi un producer
		{

			printf("Producer %d start produce\n", getpid());
			while (1)
			{
				//printf("Writer %d wait after queue_writers_sem1\n", getpid());
				sem_wait(queue_writers_sem);
				tasks->writerCount++;
				if (tasks->writerCount == 1)
				{
					//printf("Writer %d wait after read_sem\n", getpid());
					sem_wait(read_sem);
				}
				sem_post(queue_writers_sem);

				//printf("Writer %d wait after write_sem\n", getpid());
				sem_wait(write_sem);
				if (tasks->generated_tasks >= NUMBER_OF_TASKS)
				{
					sem_post(write_sem);
					tasks->writerCount--;
					if (tasks->writerCount == 0)
						sem_post(read_sem);
					printf("Producer %d leaved\n", getpid());
					exit(0);//procesele fiu vor iesi direct
				}

				int randNumber = rand();
				tasks->queue[tasks->pos_empty] = randNumber;
				tasks->pos_empty++;
				if (tasks->pos_empty == QUEUE_SIZE)
					tasks->pos_empty = 0;
				tasks->generated_tasks++;
				printf("Writer %d created %d at position [%d]\n", getpid(), randNumber, tasks->pos_empty - 1);
				sem_post(write_sem);

			
				//printf("Writer %d wait after queue_writers_sem2\n", getpid());
				sem_wait(queue_writers_sem);
				tasks->writerCount--;
				if (tasks->writerCount == 0)
				{
					//printf("Writer %d wait after read_sem\n", getpid());
					sem_post(read_sem);
				}
				sem_post(queue_writers_sem);
			}
		} 
		else if (child > 0)//parrent process
		{
			continue;//continue pentru a crea urmatorul producator
		}
		else
		{
			perror("Error to fork");
			exit(3);
		}
	}
	
	while (wait(NULL) > 0);//parintele asteapta dupa toti fii producer
	tasks->isOpen = 0;//specifica ca producatorii au terminat treaba

	sem_close(init_sem);
	sem_close(write_sem);
	sem_close(read_sem);
	sem_close(queue_writers_sem);
	sem_unlink(INIT_SEM_NAME);
	sem_unlink(WRITE_SEM_NAME);
	sem_unlink(READ_SEM_NAME);
	sem_unlink(QUEUE_WRITERS_SEM_NAME);

	printf("Main writer %d leaved\n", getpid());

	exit(0);
}
