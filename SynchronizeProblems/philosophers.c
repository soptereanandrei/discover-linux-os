// Autor: Sopterean Mihai
// Creat: 30/04/2021
// Editat:
// Problema filosofilor, programul primeste un numar N din linia de comanda, N reprezinta numarul de filosofi, fiecare folosof reprezinta un thread.
// Exista o masa cu N locuri, N farfurii si N furculite, fiecare filosof gandeste si mananca, pentru a manca are nevoie de
// furculita sa si furculita vecinului din stanga

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int N;
int *forks;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void think(int phiId)
{
	int wait = rand() % 5;
	printf("Philosopher %d thinking\n", phiId);
	sleep(wait);
}

void eat(int phiId)
{
	printf("Philosopher %d wants to eat\n", phiId);
	int posRightFork = phiId;
	int posLeftFork = phiId - 1 >= 0 ? phiId - 1 : N - 1;

	pthread_mutex_lock(&mutex);
	while (forks[posLeftFork] == 0 || forks[posRightFork] == 0)
		pthread_cond_wait(&cond, &mutex);
	forks[posLeftFork] = 0;
	forks[posRightFork] = 0;
	pthread_mutex_unlock(&mutex);

	printf("Philosopher %d is eating\n", phiId);
	int wait = rand() % 5;
	sleep(wait);

	pthread_mutex_lock(&mutex);
	forks[posLeftFork] = 1;
	forks[posRightFork] = 1;
	pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&mutex);

	printf("Philosopher %d finished to eat\n", phiId);
}

void *philosophy(void *id)
{
	long phiId = (long)id;

	while (1)
	{
		think(phiId);
		eat(phiId);
	}

	return NULL;
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s N\n", argv[0]);
		exit(1);
	}

	if (sscanf(argv[1], "%d", &N) < 1)
	{
		fprintf(stderr, "Invalid N\n");
		exit(2);
	}

	forks = (int*)malloc(sizeof(int) * N);
	for (int i = 0; i < N; i++)
		forks[i] = 1;

	pthread_t t;
	for (int i = 0; i < N; i++)
	{
		pthread_create(&t, NULL, philosophy, (void*)i);
	}

	while(1);
	
	return 0;
}
