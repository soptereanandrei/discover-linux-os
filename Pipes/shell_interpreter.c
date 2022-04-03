// Autor: Sopterean Mihai
// Creat: 19/05/2021
// Editat: 20/05/2021
// Programul implementeaza un interpretor de comenzi shell care are functia de pipe intre acestea

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

typedef struct _comand {
	char *command_name;
	char **options;
}COMMAND;

void printCommands(COMMAND **commands)
{
	for (int i = 0; commands[i]; i++)
	{
		printf("%s ", commands[i]->command_name);
		if (commands[i]->options)
		{
			char **option = commands[i]->options;
			while (*option)
			{
				printf("%s ", *option);
				option++;
			}
		}
		printf("\n");
	}
}

void printPipe(int *pipe)
{
	printf("Pipe content:\n");
	char buffer[101];
	int ret;
	while ((ret = read(pipe[0], buffer, 100)) > 0)
	{
		buffer[ret] = '\0';
		fprintf(stderr, "%s", buffer);
	}
}



//length - returneaza lungimea array-ului creat
COMMAND **getCommands(char *inputBuffer, int *setSize)
{
	//creaza array de comenzi
	int size = 0;

	char **tmp = (char**)malloc(sizeof(char*));//retine pointeri la fiecare inceput de comanda din inputBuffer
	char *command = strtok(inputBuffer, "|\n");
	while (command)
	{
		int index = size++;
		tmp = (char**)realloc(tmp, size * sizeof(char*));
		tmp[index] = command;

		command = strtok(NULL, "|\n");
	}

	COMMAND **commands = (COMMAND**)malloc((size + 1) * sizeof(COMMAND*));//retine pointeri la structuri
	commands[size] = NULL;
	for (int i = 0; i < size; i++)
	{
		commands[i] = (COMMAND*)malloc(sizeof(COMMAND));

		char *name = strtok(tmp[i], " ");
		commands[i]->command_name = name;

		int nrOptions = 1;
		commands[i]->options = (char**)malloc(2 * sizeof(char*));
		commands[i]->options[0] = name;
		commands[i]->options[1] = NULL;

		char *option = strtok(NULL, " ");
		while (option)
		{
			int index = nrOptions++;
			commands[i]->options = (char**)realloc(commands[i]->options, (nrOptions + 1) * sizeof(char*));
			commands[i]->options[index] = option;
			commands[i]->options[index + 1] = NULL;

			option = strtok(NULL, " ");
		}
	}

	*setSize = size;
	return commands;
}


//command - pointer to command
//pipes - pipes
//i - position in pipe
//size - commands count
void execute(COMMAND *command, int **pipes, int i, int size)
{
	//fiul

	if (i > 0)//intre (1, n) mapeaza intrare standard din pipe-ul din stanga lui
	{
		dup2(pipes[i-1][0], 0);//stdin <=> pipes[i-1][0]
	}

	if (i < size - 1)//intre (0, n-1) mapeaza iesirea standard spre pipe-ul din dreapta lui
	{
		dup2(pipes[i][1], 1);//stdout <=> pipes[i][1]
	}

	//pentru fiecare process fiu trebuie inchisi toti file descriptorii la pipe-uri
	//o sa ramana deschisi doar fd 0 si fd 1, la care sunt mapate pipe-uri vecine sau stdin/stdout in functie de caz
	for (int p = 0; p < size - 1; p++)
	{
		close(pipes[p][0]);
		close(pipes[p][1]);
	}
		
	execvp(command->command_name, command->options);
	perror("Error to execv");
}

void executePipe(COMMAND **commands, int size)
{
	int **pipes = (int**)malloc((size - 1) * sizeof(int*));
	for (int i = 0; i < size - 1; i++)
	{
		pipes[i] = (int*)malloc(sizeof(int) * 2);
		if (pipe(pipes[i]) < 0)
		{
			perror("error to create pipe");
		}
	}

	for (int i = 0; i < size; i++)
	{
		pid_t pid = fork();

		if (pid == 0)//child
		{
			if (size > 1)
				execute(commands[i], pipes, i, size);
			else
				execvp(commands[i]->command_name, commands[i]->options);
		}
		else if (pid > 0)//parent
		{
			continue;
		}
		else
			perror("fork error");
	}

	//dupa ce a lansat in executie procesele fiu, parintele isi inchide propii file descriptori la pipe-uri deoarece nu le foloseste
	for (int i = 0; i < size - 1; i++)
	{
		close(pipes[i][0]);
		close(pipes[i][1]);
	}

	pid_t pid;
	int status;
	while ((pid = wait(&status)) != -1)
		;//fprintf(stderr, "process %d exits with %d\n", pid, WEXITSTATUS(status));
	
	for (int i = 0; i < size - 1; i++)
		free(pipes[i]);
	free(pipes);
}

int main(void)
{
	char inputBuffer[101];
	int ret;

	while (1)
	{
		write(1, ":>", 2);

		if ((ret = read(0, &inputBuffer, 100)) < 0)
		{
			perror("errot to read input");
			exit(1);
		}

		int size;
		COMMAND **commands = getCommands(inputBuffer, &size);
		//printCommands(commands);

		executePipe(commands, size);
		//execvp(commands[0]->command_name, commands[0]->options);
		//
		for (int i = 0; i < size; i++)
			free(commands[i]);
		free(commands);
	}

}

