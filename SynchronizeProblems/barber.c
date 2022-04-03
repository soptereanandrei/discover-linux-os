// Autor: Sopterean Mihai
// Creat: 30/04/2021
// Editat:
// Programul simuleaza o frizerie in care exista un singur frizer si N scaune de asteptare.
// Un singur client poate fi servit intr-un anumit moment si maxim N clienti pot astepta in frizerie.
// Atat frizerul cat si clientii sunt diferite thread-uri

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define GENERATED_CLIENTS 10

typedef struct _Client {
	pthread_t id;
	pthread_cond_t finished;
}CLIENT;

int N;
CLIENT **queue;//coada de asteptare din frizerie
int numberOfClients;//numarul de clienti care asteapta
int first;//primul la coada
int last;//urmatorul scaun liber

CLIENT clients[GENERATED_CLIENTS];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queueWait = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;

void *barber(void *i)
{
	CLIENT *client;

	while (1)
	{
		pthread_mutex_lock(&mutex);
		while (numberOfClients == 0)
		{
			printf("The barber shop is empty, the barber will sleep\n");
			pthread_cond_wait(&empty, &mutex);
		}
		client = queue[first];
		first++;
		if (first >= N)
			first = 0;
		numberOfClients--;
		printf("Take client %ld\nQueue size = %d\n", client->id, numberOfClients);
		pthread_cond_signal(&queueWait);
		pthread_mutex_unlock(&mutex);
	
		//printf("Triming client %ld\n", client->id);
		int trimTime = rand() % 5;
		sleep(trimTime);

		printf("Finished trim client %ld\n", client->id);
		if (pthread_cond_signal(&client->finished) != 0)
			perror("Error to signal client");
	}

	return NULL;
}

void *client(void *c)
{
	CLIENT *client = (CLIENT*)c;

	printf("Client %ld arrived to barber shop\n", client->id);

	pthread_mutex_lock(&mutex);
	while (numberOfClients >= N)
		pthread_cond_wait(&queueWait, &mutex);
	queue[last] = client;
	last++;
	if (last >= N)
		last = 0;
	numberOfClients++;
	printf("Client %ld entered in queue\nQueue size = %d\n", client->id, numberOfClients);
	pthread_cond_signal(&empty);

	pthread_cond_wait(&client->finished, &mutex);
	pthread_mutex_unlock(&mutex);
	printf("Client %ld leaving\n", client->id);
	
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

	queue = (CLIENT**)malloc(sizeof(CLIENT*) * N);
	pthread_t b;
	pthread_create(&b, NULL, barber, NULL);

	for (int i = 0; i < GENERATED_CLIENTS; i++)
	{
		int wait = rand() % 2;
		sleep(wait);

		pthread_cond_init(&clients[i].finished, NULL);
		pthread_create(&(clients[i].id), NULL, client, &clients[i]);
	}

	for (int i = 0; i < GENERATED_CLIENTS; i++)
	{
		pthread_join(clients[i].id, NULL);
	}
	free(queue);

	printf("Main thread finished\n");
	return 0;
}
