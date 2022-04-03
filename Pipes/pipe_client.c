// Autor: Sopterean Mihai
// Creat: 10/05/2021
// Editat:
//
// Programul citeste din linia de comanda 2 operanzi si un operator, trimite informatiile procesului server
// folosind un pipe, procesorul server realizeaza operatia si o trimite inapoi la procesorul client

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

char EXPRESION_PIPE_NAME[] = "expressionPipe";
char RESULT_PIPE_NAME[] = "resultPipe";
int expressionPipe;
int resultPipe;

int main()
{
	if (mkfifo(EXPRESION_PIPE_NAME, 0600) < 0)
	{
		perror("Error create expressionPipe");
		exit(1);
	}

	if (mkfifo(RESULT_PIPE_NAME, 0600) < 0)
	{
		perror("Error create resultPipe");
		exit(2);
	}


	char buffer[101];
	while(1)
	{
		write(1, ">", 1);
		int ret;
		if ((ret = read(0, buffer, 100)) < 0)
		{
			perror("Error write from console");
			continue;
		}
		buffer[ret] = '\0';

		char *nr1 = strtok(buffer, " \n");
		if (nr1 == NULL)
			goto print_error;

		char *nr2 = strtok(NULL, " \n");
		if (nr2 == NULL)
			goto print_error;

		char *op = strtok(NULL, " \n");
		if (op == NULL)
			goto print_error;
		
		pid_t childPid = fork();
		if (childPid > 0) //parent
		{
			if ((expressionPipe = open(EXPRESION_PIPE_NAME, O_WRONLY)) < 0)
			{
				perror("Error open expressionPipe");
				exit(3);
			}
			int nr1Val;
			if (sscanf(nr1, "%d", &nr1Val) < 1)
			{
				fprintf(stderr, "Invalid number1\n");
			}
			int nr2Val;
			if (sscanf(nr2, "%d", &nr2Val) < 1)
			{
				fprintf(stderr, "Invalid number2\n");
			}

			if (write(expressionPipe, (char*)&nr1Val, sizeof(int)) < 0)
			{
				perror("Client error to write to expressionPipe");
				close(expressionPipe);
				exit(4);
			}
			if (write(expressionPipe, (char*)&nr2Val, sizeof(int)) < 0)
			{
				perror("Client error to write to expressionPipe");
				close(expressionPipe);
				exit(4);
			}
			if (write(expressionPipe, op, sizeof(char)) < 0)
			{
				perror("Client error to write to expressionPipe");
				close(expressionPipe);
				exit(4);
			}

			if ((resultPipe = open(RESULT_PIPE_NAME, O_RDONLY)) < 0)
			{
				perror("Error to open resultPipe");
				close(expressionPipe);
				exit(5);
			}
			close(expressionPipe);
			
			int res;
			if (read(resultPipe, (char*)&res, sizeof(int)) < sizeof(int))
			{
				perror("Error to read from resultPipe");
				close(expressionPipe);
				close(resultPipe);
			}
			
			printf("Result = %d\n", res);

			//close(expressionPipe);
			close(resultPipe);
			continue;	
		}
		else if (childPid == 0)//child
		{
			execl("./server", "./server", NULL);
			perror("Error call execv");
		}
		else
		{
			perror("Error create server");
		}

print_error:
		fprintf(stderr, "Invalid parameters, usage: number1 number2 [+-]\n");
	
	}

	return 0;
}
