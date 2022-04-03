// Autor: Sopterean Mihai
// Creat: 18/04/2021
// Editat:
// Programul calculeaza histograma pe 1,2,4,8 biti multithreading dintr-un fisier
// Se stabileste un numar de thread-uri pentru care se imparte egal fisierul, fiecare thread calculeaza separat intr-o structura
// separata histograma pe intervalul sau din fisier

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>

#define BILLION 1000000000L
#define NR_THREADS 4

typedef struct _THREAD_STRUCT {
	//int fd;//propriul file descriptor pentru a avea indepedent cursorul
	int start;//pozitia din fisier la care incepe
	int end;//pozitia din fisier la care se termina
	int n1[2];//histograma pe 1 bit
	int n2[4];//histograma pe 2 biti
	int n4[16];//histograma pe 4 biti
	int n8[256];//histograma pe 8 biti
} THREAD_STRUCT;

pthread_t thread_id[NR_THREADS];
THREAD_STRUCT thread_struct[NR_THREADS];
char *fileName;

// n - numarul de biti pe care se face histograma
// container - adresa sirului de int care retine histograma pe n biti
// buffer - adresa sirului de octeti pe care se realizeaza histograma
// len - lungimea sirului de octeti
void byteHistogram(int n, int *container, unsigned char *buffer, int len)
{
	unsigned char mask = (1 << n) - 1;
	unsigned char match;
	unsigned char byte;
	//parcurge fiecare octet
	for (int i = 0; i < len; i++)
	{
		byte = buffer[i];
		for (int j = 0; j < 8 / n; j++)
		{
			match = (byte >> j * n) & mask;
			container[match]++;
		}
	}
}

void *histogram(void *th_struct)
{
	THREAD_STRUCT *thread_s = (THREAD_STRUCT*)th_struct;

	int fd;
	if ((fd = open(fileName, O_RDONLY)) < 0)
	{
		perror("Error open file");
		exit(4);
	}
	if (lseek(fd, thread_s->start, SEEK_SET) < 0)
	{
		perror("Error set cursor start to interval");
		exit(5);
	}

	unsigned char buffer[512];
	int start = thread_s->start;
	int end = thread_s->end;
	int block_len;
	do
	{
		block_len = start + 512 < end ? 512 : end - start;
		start = start + block_len;
		if (read(fd, buffer, block_len) != block_len)
		{
			perror("Error at read from file");
		}
		byteHistogram(1, &(thread_s->n1[0]), buffer, block_len);
		byteHistogram(2, &(thread_s->n2[0]), buffer, block_len);
		byteHistogram(4, &(thread_s->n4[0]), buffer, block_len);
		byteHistogram(8, &(thread_s->n8[0]), buffer, block_len);
	} while (start < end);

	return NULL;
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		exit(1);
	}

	fileName = argv[1];
	int fd = open(fileName, O_RDONLY);
	if (fd < 0)
	{
		perror("Cannot open file");
		exit(2);
	}
	int fileLength = lseek(fd, 0, SEEK_END);
        if (fileLength < 0)
	{
		perror("Error to read file length");
		exit(3);
	}	
	close(fd);

	struct timespec timeStart;
	struct timespec timeFinish;

	if (clock_gettime(CLOCK_MONOTONIC, &timeStart) < 0)
	{
		perror("Get time error");
		exit(4);

	}

	int bytesPerThread = fileLength / NR_THREADS;
	if (bytesPerThread == 0)
		bytesPerThread = 1;

	int start = 0;
	int end = bytesPerThread;
	int i;
	for (i = 0; i < NR_THREADS && start <= fileLength; i++)
	{
		if (i == NR_THREADS - 1)
			end = fileLength;
		thread_struct[i].start = start;
		thread_struct[i].end = end;

		pthread_create(&thread_id[i], NULL, histogram, &thread_struct[i]);	

		start = end;
		end = start + bytesPerThread;
	}
	THREAD_STRUCT *sum = (THREAD_STRUCT*)calloc(sizeof(THREAD_STRUCT), 1);

	for (int j = 0; j < i; j++)
	{
		pthread_join(thread_id[j], NULL);
		for (int k = 0; k < 2; k++)
			sum->n1[k] += thread_struct[j].n1[k];
		for (int k = 0; k < 4; k++)
			sum->n2[k] += thread_struct[j].n2[k];
		for (int k = 0; k < 16; k++)
			sum->n4[k] += thread_struct[j].n4[k];
		for (int k = 0; k < 256; k++)
			sum->n8[k] += thread_struct[j].n8[k];

	}

	if (clock_gettime(CLOCK_MONOTONIC, &timeFinish) < 0)
	{
		perror("Get time error");
		//exit(4);
	}

	uint64_t diff = BILLION * (timeFinish.tv_sec - timeStart.tv_sec) + timeFinish.tv_nsec - timeStart.tv_nsec;
	int aproxMem = NR_THREADS * (2 + 4 + 16 + 256 + 1024) / 1024;
	printf("Do histogram with %d threads in: %luns with aprox memory: %dMB\n", NR_THREADS, diff, aproxMem);

	int n = 1;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < (1 << n); j++)
		{
			switch (i)
			{
				case 0:
					printf("n%d=%d ", j, sum->n1[j]);
					break;
				case 1:
					printf("n%d=%d ", j, sum->n2[j]);
					break;
				case 2:
					printf("n%d=%d ", j, sum->n4[j]);
					break;
				case 3:
					printf("n%d=%d ", j, sum->n8[j]);
					break;
			}
		}
		printf("\n------------\n");
		n *= 2;
	}



	return 0;
}
