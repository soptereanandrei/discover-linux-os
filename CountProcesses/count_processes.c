// Autor: Sopterean Mihai
// Creat: 04/04/2021
// Editat:
// Programul contorizeaza cate procese se creeaza

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{
	pid_t root = getpid();
	
	int p[2];

	if (pipe(p) < 0)
	{
		perror("Error create pipe");
		exit(1);
	}
	
	int contor = 0;

	for (int i = 1; i <= 10; i++)
	{
		pid_t child = fork();
		if (child == 0) //child
		{
			continue;
		}
		else if (child > 0) //parent
		{
			wait(NULL);
			read(p[0], (char*)&contor, sizeof(int));
			//printf("contor = %d\n", contor);
		}
		else
		{
			perror("fork error");
			exit(2);
		}
	}

	contor = contor + 1;
	write(p[1], (char*)&contor, sizeof(int));

	if (getpid() == root)
		printf("%d \n", contor);

	return 0;
}
