// Autor: Sopterean Mihai
// Creat: 12/04/2021
// Editat:
// Programul citeste dintr-un fisier text numit numbers.in cate o linie, iar pentru fiecare linie creeaza cate un thread care face medie
// aritmetica si afiseaza rezultatul intr-un fisier, de asemenea threadurile returneaza rezultatul prin intermediul pthread_exit
//

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct _THREAD
{
	pthread_t id;
	struct _THREAD *next;
}THREAD;

THREAD *first, *last;

typedef struct _INPUT
{
	int n1;
	int n2;
	int fd;
}INPUT;

void addThread(THREAD *thread)
{
	if (first == NULL)
	{
		first = last = thread;
		return;
	}
	last->next = thread;
	last = thread;
}

void *average(void *in)
{
	INPUT *input = (INPUT*)in;

	int n1 = input->n1;
	int n2 = input->n2;
	int fd = input->fd;

	float average = (n1 + n2) / 2.0f;

	char output[200];
	sprintf(output, "%d %d %.2f tid=%ld\n", n1, n2, average, pthread_self());

	if (write(fd, output, strlen(output)) < 0)
	{
		perror("Error to write to result_threads.out");
	}

	free(input);
	long container;
	memcpy((char*)&container, (char*)&average, sizeof(long));
	pthread_exit((void*)container);
}

int main(void)
{
	int fd_read = open("numbers.in", O_RDONLY);
	if (fd_read < 0)
	{
		perror("Error open numbers.in");
		exit(1);
	}

	int fd_threads_results = open("result_threads.out", O_CREAT | O_TRUNC | O_WRONLY, 0644);
	if (fd_threads_results < 0)
	{
		perror("Error create result_threads.out");
		exit(2);
	}

	int fd_main_results = open("results_main.out", O_CREAT | O_TRUNC | O_WRONLY, 0644);
	if (fd_main_results < 0)
	{
		perror("Error create results_main.out");
		exit(3);
	}

	int ret;
	char buffer[31];
	while ((ret = read(fd_read, buffer, 30)) > 0)
	{
		buffer[ret] = 0;

		int i = 0;
		while (buffer[i] != 0 && buffer[i] != '\n')
			i++;
		buffer[i] = 0;
		if (i != ret)//verifica daca '\n' se afla la sfarsitul buffer-ului
			if (lseek(fd_read, -(ret - i - 1), SEEK_CUR) < 0)//setez pointerul la inceputul noii linii
				perror("Error lseek");
		
		int n1, n2;
		char *number = strtok(buffer, " ");
		if (number)
		{
			if (sscanf(number, "%d", &n1) < 1)
			{
				perror("Error to read first number");
			}
		}
		else
			continue;

		number = strtok(NULL, " ");
		if (number)
		{
			if (sscanf(number, "%d", &n2) < 1)
			{
				perror("Error to read second number");
			}
		}
		else
			continue;
		
		INPUT *input = (INPUT*)malloc(sizeof(INPUT));
		input->n1 = n1;
		input->n2 = n2;
		input->fd = fd_threads_results;

		THREAD *newThread = (THREAD*)malloc(sizeof(THREAD));
		pthread_create(&(newThread->id), NULL, average, (void*)input);
		newThread->next = NULL;

		addThread(newThread);
	}

	char output[200];
	THREAD *tmp = first;
	while (tmp)
	{
		float average;
		if (pthread_join(tmp->id, (void*)&average) < 0)
		{
			perror("Error join thread");
		}
		
		sprintf(output, "%.2f tid=%ld\n", average, tmp->id);
		if (write(fd_main_results, output, strlen(output)) < 0)
		{
			perror("Error write to results_main.out");
		}

		tmp = tmp->next;
	}	

	close(fd_read);
	close(fd_threads_results);
	close(fd_main_results);

	return 0;
}
