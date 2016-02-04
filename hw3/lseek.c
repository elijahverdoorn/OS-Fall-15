// Elijah Verdoorn, CSCI 273, Dick Brown, St. Olaf College, Fall 2015

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main (int argc, char **argv, char **envp)
{
	int fd = open("test.txt", 0);
	char buff[100];
	read(fd, buff,10);
	printf("bytes were read:  %s\n", buff);
	lseek(fd, 0, 0);
	read(fd, buff,10);
	printf("bytes were read:  %s\n", buff);
	lseek(fd, 100, 0);
	read(fd, buff,10);
	printf("bytes were read:  %s\n", buff);

}