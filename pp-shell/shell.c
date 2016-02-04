// Elijah Verdoorn, CSCI 273, Dick Brown, St. Olaf College, Fall 2015

#include <stdio.h> // for IO
#include <ctype.h> // for isspace()
#include <stdlib.h> // standard lib
#include <string.h> // for strcpy();
#include <unistd.h> 
#include <sys/wait.h> // for wait() for parent process
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h> // for bool datatype

#define MAXTOKS 100 // the maximum number of tokens allowed in a line of commands
#define MAXFILES 100 // the maximum number of files allowed for redirection per line of command

enum status_value {NORMAL, EOF_OR_ERROR, TOO_MANY_TOKENS};

struct name
{
	char** tok; //hold "tokens" AKA strings
	int count; //hold number of words
	int fileCount; // hold number of files
	int status; //hold info about status of parsing
	int hasRedirect;
	char** fileName;
};

//parsing function
//arg is the address of a name struct
//it changes the status of the name struct based on success or failure rather than using a return value
int read_name(struct name *arg)
{
	arg->tok = malloc(sizeof(char *) * MAXTOKS); // allocate all the TOKs
	arg->fileName = malloc(sizeof(char *) * MAXFILES); // allocate the file (we might not use them)
	char* getlineBuff = NULL;
	size_t getlineLen = NULL;

	arg->status = NORMAL;

	int numCharsInLine = getline(&getlineBuff, &getlineLen, stdin); // get a line of commands

	int numCharsInTok = 0;
	int startingPlaceInBuff = 0;

	while(isspace(getlineBuff[startingPlaceInBuff])) // handle whitespace at the start of an input
	{
		startingPlaceInBuff++;
	}

	while(startingPlaceInBuff < numCharsInLine)
	{
		while(!isspace(getlineBuff[startingPlaceInBuff + numCharsInTok]) && startingPlaceInBuff + numCharsInTok <= numCharsInLine)	
		{
			numCharsInTok++; // count how many chars in this tok
		}

		arg->tok[arg->count] = malloc((sizeof(char) * numCharsInTok) + 1); //allocate memory for this tok
		int i = 0; // counting variable
		int j = 0;

		for (j; j < numCharsInTok; j++)
		{
			arg->tok[arg->count][j] = getlineBuff[startingPlaceInBuff + j]; // copy the tok into the struct
		}
		//append the array with a nullbyte
		arg->tok[arg->count][j + 1] = '\0';

		arg->count++;
		startingPlaceInBuff = startingPlaceInBuff + numCharsInTok;
		numCharsInTok = 0;
		while(isspace(getlineBuff[startingPlaceInBuff])) // handle multiple consecutive whitespace characters
		{
			startingPlaceInBuff++;
		}

		if (arg->count + 1 == MAXTOKS) // handle too many toks
		{
			int numCharsExtra = numCharsInLine - startingPlaceInBuff;
			arg->tok[arg->count] = malloc((sizeof(char) * numCharsExtra) + 1);
			int i = 0; // counting variable
			int j = 0;
			for (j; j < numCharsExtra; j++)
			{
				arg->tok[arg->count][j] = getlineBuff[startingPlaceInBuff + j]; // copy the tok into the struct
			}
			//append the array with a nullbyte
			arg->tok[arg->count][j + 1] = '\0';
			arg->status = TOO_MANY_TOKENS;
			break;
		}
	}

	if (arg->status == NORMAL) //return correct value
	{
		return 1;
	} else {
		return 0;		
	}
}

//allows for changing directory within the shell
int change_directory(const char* filePath)
{
	int ret = 0;
	ret = chdir(filePath);
	return ret;
}

// allows for the use of > and < characters to redirect IO
// arg is the address of a name struct
// returns 0 no matter what, it should be changed in the future to return values based on success or error
int parse_redirection(struct name *arg)
{
	int ret = 0;
	int i = 0;
	for (i; i < arg->count; i++)
	{
		int j = 0;
		int numCharsInTok = 0;

		switch (arg->tok[i][0])
		{
			case '>':
				// need to copy the next tok to the filename
				arg->hasRedirect = 1;
				for (numCharsInTok; arg->tok[i + 1][numCharsInTok] != '\0'; numCharsInTok++); // count the number of characters in the file name
				arg->fileName[0] = malloc((sizeof(char) * numCharsInTok) + 1); //allocate memory for this tok
				for (j; arg->tok[i +1 ][j] != '\0'; j++)
				{
					arg->fileName[0][j] = arg->tok[i + 1][j];
				}
				arg->fileName[0][numCharsInTok + 1] = '\0'; // append with nullbyte
				//delete the rest so that we don't try to execute it
				arg->tok[i] = '\0';
				arg->tok[i + 1] = '\0';
				return 0;
				break;
			case '<':
				// need to copy the next tok to the filename
				arg->hasRedirect = 2;
				for (numCharsInTok; arg->tok[i + 1][numCharsInTok] != '\0'; numCharsInTok++); // count the number of characters in the file name
				arg->fileName[0] = malloc((sizeof(char) * numCharsInTok) + 1); //allocate memory for this tok
				for (j; arg->tok[i +1 ][j] != '\0'; j++)
				{
					arg->fileName[0][j] = arg->tok[i + 1][j];
				}
				arg->fileName[0][numCharsInTok + 1] = '\0'; // append with nullbyte
				//delete the rest so that we don't try to execute it
				arg->tok[i] = '\0';
				arg->tok[i + 1] = '\0';
				return 0;
				break;
		}
	}
}

int main(int argc, char const *argv[], char** envp)
{
	printf("Welcome to Elijah's Shell.\n");
	while (1)
	{
		struct name myName = {0, 0, 0, 0, -1, 0};
		printf("EShell: ");
		read_name(&myName);
		parse_redirection(&myName);

		if (myName.tok[0][0] == 'c' && myName.tok[0][1] == 'd') // Implementation of 'cd' command
		{
			// CD Detected
			if (myName.tok[1] != '\0')
			{
			  if (change_directory(myName.tok[1]) == -1) // change the directory
				{
					printf("Error: chdir() returned an error. Check if your path is valid.\n");
				}
			} else {
				printf("Error: please specify a path when using 'cd'\n");
			}
		} else { //no 'cd' detected, continue
			if (myName.tok[0][0] == 'e' && myName.tok[0][1] == 'x' && myName.tok[0][2] == 'i' &&myName.tok[0][3] == 't')
			{
				printf("Exit command recieved. Quitting.\n");
				return 0;
			} else {
				int ret;  // for return value from fork()
				int status;
				if ((ret = fork()) < 0)
				{
					printf("fork() FAILED");
					perror("forkeg");
					_exit(1);
				}
				if (ret != 0)
				{
					// parent process
					waitpid(ret, &status, WUNTRACED | WCONTINUED);
				} else {
				  	// child process
					int execRet;
					int fd;
					switch(myName.hasRedirect)
					{
						case -1: // no redirection
							execRet = execve(myName.tok[0],myName.tok, envp);
							perror("execve");
							_exit(1);
							break;
						case 1: // > 
							fd = open(myName.fileName[0], O_WRONLY | O_CREAT, 0666);
							dup2(fd,1);
							close(fd);
							execRet = execve(myName.tok[0], myName.tok, envp);
							perror("execve");
							_exit(1);
						break;
						case 2: // <

							fd = open(myName.fileName[0], O_RDONLY);
							dup2(fd,STDIN_FILENO);
							close(fd);
							execRet = execve(myName.tok[0], myName.tok, envp);
							perror("execve");
							_exit(1);
						break;
					}
				}
			}
		}
	}
	return 0;
}
