// Elijah Verdoorn, CSCI 273, Dick Brown, St. Olaf College, Fall 2015

#include <stdio.h>
#include <stdlib.h>

int main (int argc, char **argv, char **envp)
{
	printf("Printing the entire enviornment.\n");
	int i = 0;
	//using the third arguement to print the enviornment
	for (i = 0; envp[i] != NULL; i++)
	{
		printf(envp[i]);
		printf("\n");
	}
}