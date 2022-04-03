// Autor: Sopterean Mihai
// Creat: 05/04/2021
// Editat:
//
// Programul primeste un sablon si un numar de fisiere din linia de comanda in care cauta sablonul, programul face comparatia intre o cautare single thread si multi thread

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>

#define BILLION 1000000000L

#define NR_THREADS 5

pthread_t threads[NR_THREADS];

typedef struct _FILE_INFOS {
	char **filesPaths;
	int count;
} FILE_INFOS;

FILE_INFOS fileInfos[NR_THREADS];

char *sablon;

//filePath - pointer la un sir de caractere care reprezinta calea absoluta spre fisiere in care se doreste cautarea
//funtia returneaza numarul de aparatii a sablonului in fisier
void *search_word(void *in)
{
	FILE_INFOS *filesInfos = (FILE_INFOS*)in;
	char **filePath = filesInfos->filesPaths;
       	int count = filesInfos->count;	

	long nr = 0;
	for (int i = 0; i < count; i++)
	{
		int fd;

		if ((fd = open(filePath[i], O_RDONLY)) < 0)
		{
			perror("Cannot open file");
			return (void*)-1;
		}

		char buffer[513];

		int sablon_len = strlen(sablon);

		int ret;
		while ((ret = read(fd, buffer, 512)) > 0)
		{
			char *cFind;
			char *strFind;
			int offset = 0;

			buffer[ret] = '\0';
			while ((cFind = strchr(buffer + offset, sablon[0])) != NULL)//cauta apartia primul caracter
			{
				int lenToEnd = strlen(cFind);
				if (ret == 512 && lenToEnd < 8) 
					//ar putea fi un cuvant trunchiat, se citeste din nou din fisier
				{
					//se muta cursorul inaintea cuvantului posibil trunchiat
					if (lseek(fd, -lenToEnd, SEEK_CUR) < 0)
					{
						perror("Cannot move seek");
						return (void*)-1;
					}
					break;
				}
				strFind = strstr(cFind, sablon);//verifica daca este intreg cuvantul
				if (strFind != NULL)
				{
					nr++;
					offset = strFind - buffer + sablon_len;
				}
				else
				{
					offset = offset + 1;
				}
			}
		}
		close(fd);
	}

	return (void*)nr;
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		fprintf(stderr, "Usage: %s sablon file1 [file2 ...]\n", argv[0]);
		exit(1);
	}

	sablon = argv[1];

	printf("Start to search the files with the main thread\n");

	struct timespec timeStart;
	struct timespec timeFinish;

	if (clock_gettime(CLOCK_MONOTONIC, &timeStart) < 0)
	{
		perror("Get time error");
		exit(2);
	}

	long contor1 = 0;
	for (int i = 2; i < argc; i++)
	{
		fileInfos[0].filesPaths = argv + i;
		fileInfos[0].count = 1;
		contor1 += (long)search_word((void*)&fileInfos[0]);	
	}

	if (clock_gettime(CLOCK_MONOTONIC, &timeFinish) < 0)
	{
		perror("Get time error");
		exit(2);
	}
	uint64_t diff;

	diff = BILLION * (timeFinish.tv_sec - timeStart.tv_sec) + timeFinish.tv_nsec - timeStart.tv_nsec;
	printf("Search with main thread finished, time elapsed = %llu ns with Contor1 = %ld\n", 
			(long long unsigned int)diff,
			contor1
	      );

	printf("Start searching with %d threads\n", NR_THREADS);

	if (clock_gettime(CLOCK_MONOTONIC, &timeStart) < 0)
	{
		perror("Get time error");
		exit(2);
	}

	int nr_files = argc - 2;
	int nr_file_per_thread = nr_files / NR_THREADS;
	if (nr_file_per_thread == 0)
		nr_file_per_thread = 1;

	int i;
	int offset = 0;
	int count;
	for (i = 0; i < NR_THREADS && i < nr_files; i++)
	{
		offset = nr_file_per_thread * i;
		if (i < NR_THREADS - 1)
			count = nr_file_per_thread;
		else
			count = nr_files - offset;

		fileInfos[i].filesPaths = argv + 2 + offset;
		fileInfos[i].count = count;

		if (pthread_create(&threads[i], NULL, search_word, (void*)&fileInfos[i]) < 0)
		{
			perror("Error create thread");
			exit(3);
		}
	}

	long contor2 = 0;
	for (int j = 0; j < i; j++)
	{
		long tmp;
		pthread_join(threads[j], (void*)&tmp);
		contor2 += tmp;
	}

	if (clock_gettime(CLOCK_MONOTONIC, &timeFinish) < 0)
	{
		perror("Get time error");
		exit(2);
	}

	diff = BILLION * (timeFinish.tv_sec - timeStart.tv_sec) + timeFinish.tv_nsec - timeStart.tv_nsec;
	printf("Search with %d threads finished, time elapsed = %llu ns with Contor2 = %ld\n", 
			NR_THREADS,
			(long long unsigned int)diff,
			contor2
	      );

	return 0;
}
