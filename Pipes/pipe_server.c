// Autor: Sopterean Mihai
// Creat: 10/05/2021
// Editat:
//
// Programul primeste printr-un pipe o expresie de forma op1 op2 operator si calculeaza expresia, 
// intoarce rezultatul expresiei printr-un pipe procesului client care a trimis expresia

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char EXPRESION_PIPE_NAME[] = "expressionPipe";
char RESULT_PIPE_NAME[] = "resultPipe";
int expressionPipe;
int resultPipe;

int main()
{
	if ((expressionPipe = open(EXPRESION_PIPE_NAME, O_RDONLY)) < 0)
	{
		perror("Server error to open expresionPipe");
		exit(1);
	}

	int n1, n2;
	char operator;
	
	if (read(expressionPipe, (char*)&n1, sizeof(int)) < sizeof(int))
	{
		perror("Server error to read from expressionPipe");
		close(expressionPipe);
		exit(2);
	}

	if (read(expressionPipe, (char*)&n2, sizeof(int)) < sizeof(int))
	{
		perror("Server error to read from expressionPipe");
		close(expressionPipe);
		exit(2);
	}
	
	if (read(expressionPipe, &operator, sizeof(char)) < sizeof(char))
	{
		perror("Server error to read from expressionPipe");
		close(expressionPipe);
		exit(2);
	}
	close(expressionPipe);

	int res = 0;
	switch (operator)
	{
		case '+':
			res = n1 + n2;
			break;
		case '-':
			res = n1 - n2;
			break;
		default:
			fprintf(stderr, "Invalid operator\n");
			res = -1;
			break;
	}
	//printf("%d %d %d", n1, n2, res);
	
	if ((resultPipe = open(RESULT_PIPE_NAME, O_WRONLY)) < 0)
	{
		perror("Server error to open resultPipe");
		exit(2);
	}

	if (write(resultPipe, (char*)&res, sizeof(int)) < sizeof(int))
	{
		perror("Error to write to resultPipe");
		close(expressionPipe);
		close(resultPipe);
		exit(3);
	}

	//close(expressionPipe);
	close(resultPipe);
	exit(0);
}
