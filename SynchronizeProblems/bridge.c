// Autor: Sopterean Mihai
// Creat: 30/04/2021
// Editat:
// Programul implementeaza un protocol de transversare a unui pod aflat in reparatii, pe care circulatia masinilor
// este restrictionata la un singur sens. Se presupune ca podul suporta maxim N masini concomitent.
// Fiecare masina reprezinta un thread. Se folosesc thread-uri si mechanisme de sincronizare POSIX.

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SIMULATION_TIME 20
#define DIRECTION_CHANGE_TIME 5

int N;

int bridge;//valoarea reprezinta numarul de masini pe pod, semnul reprezinta directia masinilor de pe acesta
int bridgeDirection = 1;//reprezinta directia semaforului
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int sign(int val)
{
	if (val >= 0)
		return 1;
	return -1;
}

//dir - este directia in care tranverseaza masina
void *car(void *dir)
{
	long direction = (long)dir;

	pthread_mutex_lock(&mutex);
	printf("Car %ld is at entrance on bridge in direction : %ld\n", pthread_self(), direction);
	while (sign(direction) != bridgeDirection //daca directia pe care se circula acum este inversa
			|| (sign(bridge) != sign(direction) && bridge != 0) //daca inca sunt masini pe pod care inca nu au parasit podul din sens opus
			|| abs(bridge) >= N //podul este la capacitate maxima
			)
		pthread_cond_wait(&cond, &mutex);
	bridge += direction;
	pthread_mutex_unlock(&mutex);

	printf("Car %ld is on bridge in direction : %ld\n", pthread_self(), direction);
	sleep(1);

	pthread_mutex_lock(&mutex);
	bridge -= direction;
	if (bridge == 0)
		pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&mutex);

	printf("Car %ld leaved the bridge\n", pthread_self());

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
		fprintf(stderr, "Invalid N number of cars\n");
		exit(2);
	}
	
	srand(388);
	pthread_t threads[SIMULATION_TIME];
	for (int i = 0; i < SIMULATION_TIME; i++)
	{
		pthread_mutex_lock(&mutex);
		if ((i + 1) % DIRECTION_CHANGE_TIME == 0)
		{
			bridgeDirection = -bridgeDirection;
			pthread_cond_broadcast(&cond);
		}
		printf("------------------\nBridge direction = %d and cars on bridge = %d\n-------------------\n", bridgeDirection, bridge);
		pthread_mutex_unlock(&mutex);

		long dir = rand() % 2 == 0 ? -1 : 1;
		pthread_create(&threads[i], NULL, car, (void*)dir);
		sleep(1);
	}

	//Exista posibilitatea ca programul sa nu se termine deoarece raman thread-uri care asteapta schimbarea de directie
	for (int i = 0; i < SIMULATION_TIME; i++)
	{
		pthread_join(threads[i], NULL);
	}

	printf("Main thread finished\n");

	return 0;
}
