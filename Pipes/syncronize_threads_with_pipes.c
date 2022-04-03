// Autor: Sopterean Mihai
// Creat: 19/05/2021
// Editat:
// Programul incrementeaza un contor in intervalul [0, 100], folosind 2 threaduri
// t1 incrementeaza [0, 30] si [70,100], iar threadul2 [31,69] folosind pipe-uri

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

pthread_t threads[2];
int T1T2Pipe[2];
int T2T1Pipe[2];

void *t1(void *t)
{
	while (1)
	{
		int counter;
		if (read(T2T1Pipe[0], (char*)&counter, sizeof(int)) < sizeof(int))
		{
			perror("t1 error to read from pipe");
			pthread_exit((void*)1);
		}
		printf("t1 readed %d\n", counter);

		if (counter < 0 || counter > 100)
		{
			printf("Init value out of range\n");
			if (write(T1T2Pipe[1], (char*)&counter, sizeof(int)) < sizeof(int))
			{
				perror("t1 error to write");
				pthread_exit((void*)2);
			}
			goto _close;
		}


		while ((counter >= 0 && counter <= 30) || (counter >= 70 && counter < 100))
		{
			counter++;
			printf("t1 incremented counter to %d\n", counter);
		}		

		if (write(T1T2Pipe[1], (char*)&counter, sizeof(int)) < sizeof(int))
		{
			perror("t1 error to write");
			pthread_exit((void*)2);
		}
		printf("t1 writed %d\n", counter);
		
		if (counter == 100)
			goto _close;
	}

_close:
	close(T1T2Pipe[1]);
	close(T2T1Pipe[0]);	
	close(T2T1Pipe[1]);
	printf("t1 finished\n");
	return NULL;
}

void *t2(void *t)
{
	int counter;
	if (read(T1T2Pipe[0], (char*)&counter, sizeof(int)) < sizeof(int))
	{
		perror("t1 error to read from pipe");
		pthread_exit((void*)1);
	}
	printf("t2 readed %d\n", counter);

	while (counter >= 31 && counter <= 69)
	{
		counter++;
		printf("t2 incremented counter to %d\n", counter);
	}

	if (write(T2T1Pipe[1], (char*)&counter, sizeof(int)) < sizeof(int))
	{
		perror("t1 error to write");
		pthread_exit((void*)2);
	}
	printf("t2 writed %d\n", counter);

	printf("t2 finished\n");
	return NULL;
}

int main(void)
{
	if (pipe(T1T2Pipe) < 0)
	{
		perror("error to create pipe t1 to t2");
		exit(1);
	}
	if (pipe(T2T1Pipe) < 0)
	{
		perror("error to create pipe t2 to t1");
		exit(1);
	}

	int zero = 0;
	if (write(T2T1Pipe[1], (char*)&zero, sizeof(int)) < sizeof(int))
	{
		perror("error to write 0 in pipe");
		exit(2);
	}

	pthread_create(&threads[0], NULL, t1, NULL);
	pthread_create(&threads[1], NULL, t2, NULL);
	for (int i = 0; i < 2; i++)
		pthread_join(threads[i], NULL);

	int counter;
	if (read(T1T2Pipe[0], (char*)&counter, sizeof(int)) < sizeof(int))
	{
		perror("error to read counter");
		exit(3);
	}
	printf("Result = %d\n Main exited\n", counter);

	close(T1T2Pipe[0]);

	return 0;
}
