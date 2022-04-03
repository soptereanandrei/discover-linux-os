// Autor: Sopterean Mihai
// Creat: 27/04/2021
// Editat:
// Programul simuleaza crearea moleculelor de NaCl prin combinarea atomilor de Na cu cei de Cl si viceversa, 
// fiecare atom este un thread creat aleator

#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct _ATOM{
	pthread_t id;
	int order;
	struct _ATOM *next;
}ATOM;

sem_t naMutex;
sem_t clMutex;
sem_t na;
sem_t cl;

ATOM *naFirst, *naLast;//coada de Na
ATOM *clFirst, *clLast;//coada de Cl

//se presupune ca lacatul este blocat cand se apeleaza functia
void add(ATOM *atom, ATOM** first, ATOM **last)
{
	if (*first == NULL)
	{
		*first = *last = atom;
	}
	else
	{
		(*last)->next = atom;
		*last = atom;
	}
}

void fusion()
{
	sem_wait(&naMutex);
	if (naFirst == NULL)
	{
		fprintf(stderr, "Invalid atoms of Na\n");
		sem_post(&naMutex);
		pthread_exit((void*)1);
	}
	
	ATOM *Na = naFirst;
	if (naFirst == naLast)
		naFirst = naLast = NULL;
	else
		naFirst = naFirst->next;
	sem_post(&naMutex);
	
	sem_wait(&clMutex);
	if (clFirst == NULL)
	{
		fprintf(stderr, "Invalid atoms of Cl\n");
		sem_post(&clMutex);
		pthread_exit((void*)1);
	}
	
	ATOM *Cl = clFirst;
	if (clFirst == clLast)
		clFirst = clLast = NULL;
	else
		clFirst = clFirst->next;
	sem_post(&clMutex);

	if (Na->order <= Cl->order)
		printf("Na %ld fusion with Cl %ld\n", Na->id, Cl->id);
	else
		printf("Cl %ld fusion with Na %ld\n", Cl->id, Na->id);
}

void *naAtom(void *a)
{
	ATOM *atom = (ATOM*)a;

	while (1)
	{
		sem_wait(&naMutex);
		if (atom->id != naFirst->id)
		{
			sem_post(&naMutex);
			continue;
		}
		sem_post(&naMutex);

		//este primul atom de Na
		sem_post(&na);//semnaleaza ca este pregatit
		sem_wait(&cl);//asteapta atomul de Cl sa fuzioneze

		pthread_exit(NULL);
	}
	
	return NULL;
}

void *clAtom(void *a)
{
	ATOM *atom = (ATOM*)a;

	while (1)
	{
		sem_wait(&na);//astepata dupa atomul de Na
		sem_wait(&clMutex);
		if (atom->id != clFirst->id)
		{
			sem_post(&na);
			sem_post(&clMutex);
			continue;
		}
		sem_post(&clMutex);

		//este primul atom de Cl
		fusion();

		sem_post(&cl); //semnaleaza ca a fost realizata fuziunea

		pthread_exit(NULL);
	}

	return NULL;
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s numberOfThreads\n", argv[0]);
		exit(1);
	}

	int nrThreads;
	if (sscanf(argv[1], "%d", &nrThreads) < 1)
	{
		fprintf(stderr, "Error to sscanf\n");
		exit(2);
	}

	if (sem_init(&na, 0, 0) < 0)
	{
		perror("Error init na sem");
		exit(3);
	}

	if (sem_init(&cl, 0, 0) < 0)
	{
		perror("Error init cl sem");
		exit(3);
	}

	if (sem_init(&naMutex, 0, 1) < 0)
	{
		perror("Error init cl sem");
		exit(3);
	}
	
	if (sem_init(&clMutex, 0, 1) < 0)
	{
		perror("Error init cl sem");
		exit(3);
	}


	srand(15);
	for (int i = 0; i < nrThreads; i++)
	{
		int s = rand() % 2;
		ATOM *newAtom = (ATOM*)malloc(sizeof(ATOM));
		newAtom->order = i;
		newAtom->next = NULL;

		if (s == 0)
		{
			sem_wait(&naMutex);
			pthread_create(&(newAtom->id), NULL, naAtom, newAtom);
			add(newAtom, &naFirst, &naLast);
			sem_post(&naMutex);
			printf("New atom %ld of Na\n", newAtom->id);
		}
		else
		{
			sem_wait(&clMutex);
			pthread_create(&(newAtom->id), NULL, clAtom, newAtom);
			add(newAtom, &clFirst, &clLast);
			sem_post(&clMutex);
			printf("New atom %ld of Cl\n", newAtom->id);
		}
	}

	while (1)
	{
		sem_wait(&naMutex);
		if (naFirst == NULL)//astepata dupa terminarea tuturor thread-urilor create
		{
			sem_post(&naMutex);
			break;
		}
		sem_post(&naMutex);

		sem_wait(&clMutex);
		if (clFirst == NULL)//astepata dupa terminarea tuturor thread-urilor create
		{
			sem_post(&clMutex);
			break;
		}
		sem_post(&clMutex);

		sleep(1);
	}

	sem_destroy(&naMutex);
	sem_destroy(&clMutex);
	sem_destroy(&na);
	sem_destroy(&cl);

	printf("Main thread finished\n");
	return 0;
}
