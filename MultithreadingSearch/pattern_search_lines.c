// Autor: Sopterean Mihai
// Creat: 05/04/2021
// Editat:
//
// Programul primeste un sablon si un numar de fisiere din linia de comanda in care cauta sablonul si afiseaza pentru
// fiecare fisier linia/liniile in care se regaseste sablonul
// Programul are o problema, daca un cuvant se afla pe o linie care este segmentata intre 2 citiri consecutive se afiseaza doar de la inceputul cuvantului la sfarsitul liniei
// acest lucru se datoreaza algoritmului rezolvat la cealalta problema care nu a fost proiectat pentru afisarea intregii linii si nu am mai avut timp sa o adaptez pt afisarea perfecta

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define NR_THREADS 5

pthread_t threads[NR_THREADS];
int fd_tmp[NR_THREADS]; //1 file descriptor pentru fiecare thread, folosit pentru a retine temporar liniile

typedef struct _FILE_INFOS {
	int index;
	char **filesPaths;
	int count;
} FILE_INFOS;

FILE_INFOS fileInfos[NR_THREADS];

char *sablon;
char cwd[512];

//filePath - pointer la un sir de caractere care reprezinta calea absoluta spre fisiere in care se doreste cautarea
//funtia returneaza numarul de aparatii a sablonului in fisier
void *search_word(void *in)
{
	FILE_INFOS *filesInfos = (FILE_INFOS*)in;
	int index = filesInfos->index;//thread index
	char **filePath = filesInfos->filesPaths;
	int count = filesInfos->count;	


	char tmp_file_name[1024];
	sprintf(tmp_file_name, "%s/tmp%d", cwd, index);
	if ((fd_tmp[index] = open(tmp_file_name, O_CREAT | O_TRUNC | O_RDWR, 0644)) < 0)
	{
		perror("Error create file");
		exit(4);
	}

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

		sprintf(buffer, "In file %s:\n", filePath[i]);
		write(fd_tmp[index], buffer, strlen(buffer));

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

					char *last_pos_of_newline = strFind;
					while (last_pos_of_newline >= buffer && *last_pos_of_newline != '\n')
						last_pos_of_newline--;

					char *next_pos_of_newline = strFind;
					while (next_pos_of_newline < buffer + 512 && *next_pos_of_newline != '\n')
						next_pos_of_newline++;

					if (write(fd_tmp[index], last_pos_of_newline + 1, next_pos_of_newline - last_pos_of_newline) < 0)
					{
						perror("error to write");
						exit(4);
					}
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

	if (getcwd(cwd, 512) < 0)
	{
		perror("Error get current working directory");
		exit(5);
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

		fileInfos[i].index = i;
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
		
		printf("Thread with ID = %ld find %ld matches\n", threads[j], tmp);
		
		if (lseek(fd_tmp[j], 0, SEEK_SET) < 0)
		{
			perror("Error at seek");
		}
		
		char buffer[513];
		int ret;
		while ((ret = read(fd_tmp[j], buffer, 512)) > 0)
		{
			buffer[ret] = '\0';
			printf("%s", buffer);
		}
		close(fd_tmp[j]);
		char tmp_file_name[1024];
		sprintf(tmp_file_name, "%s/tmp%d", cwd, j);
		if (remove(tmp_file_name) < 0)
		{
			perror("error delete file");
		}
	}
	printf("Total matches: %ld\n", contor2);

	return 0;
}
