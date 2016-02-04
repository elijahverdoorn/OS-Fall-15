// Elijah Verdoorn, CSCI 273, Dick Brown, St. Olaf College, Fall 2015

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h> // for isspace()
#include <string.h> // for strcpy();
#include <sys/wait.h> // for wait() for parent process
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h> // for bool datatype
#include <time.h> //for the log
#include <pthread.h> // for pthreads
#include <string.h>

#define MAXBUFF 100
#define MAXTOKS 100 // the maximum number of tokens allowed in a line of commands
#define MAXFILES 100 // the maximum number of files allowed for redirection per line of command
#define NUMTHREADS 2 // the number of threads that are used in the program

enum status_value {NORMAL, EOF_OR_ERROR, TOO_MANY_TOKENS};

pthread_mutex_t qMutex;
pthread_mutex_t lMutex;
pthread_cond_t qEmpty;

struct name // part of the parsing
{
	char** tok; //hold "tokens" AKA strings
	int count; //hold number of words
	int fileCount; // hold number of files
	int status; //hold info about status of parsing
	int hasRedirect;
	char** fileName;
};

struct tdata // to pass data to a thread
{
	int socket;
	int request_id;
	FILE *flog;
	struct tdata * link;
};


struct work_queue // the queue of requests that are yet to be fulfilled
{
	struct tdata *start; /* address of first tdata in the queue, or 0 if none */
	struct tdata **end;  /* address of last link in queue, or of start if none */
} work_queue;

char *message1 = "\r\nHTTP/1.1 200 OK\r\nDate: ";
char *message2 = "\r\nConnection: close\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: ";
char *message3 = "\n\n";
char *message4 = "HTTP/1.1 400 Bad Request";
char *message5 = "HTTP/1.1 404 Not Found";
char *message6 = "HTTP/1.1 501 Not Implemented";

//parsing function
//arg is the address of a name struct
//it changes the status of the name struct based on success or failure rather than using a return value
int read_name(struct name *arg, char* buff, int numChars)
{
	arg->tok = malloc(sizeof(char *) * MAXTOKS); // allocate all the TOKs
	arg->fileName = malloc(sizeof(char *) * MAXFILES); // allocate the file (we might not use them)
	char* getlineBuff = buff;
	size_t getlineLen = NULL;

	arg->status = NORMAL;

	int numCharsInLine = numChars; // get a line of commands

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

/* addq
		 1 argument:  address of a tdata item
		 State change:  The tdata item arg1 is appended to the end of 
	work_queue, adjusting work_queue's members to reflect the new item
		 Return: none */
void addq(struct tdata *tdatap) {
	pthread_mutex_lock(&qMutex);
	if (work_queue.end == &work_queue.start)
		pthread_cond_broadcast(&qEmpty);
	*work_queue.end = tdatap;
	tdatap->link = 0;
	work_queue.end = &tdatap->link;
	pthread_mutex_unlock(&qMutex);
	return;
}

/* removeq
		 No arguments
		 State change:  If work_queue contains at least one item,
				remove the first item from work_queue, adjusting  work_queue 
				to reflect that removal. 
		 Return: The address of the removed struct tdata item, or 0 if 
				work_queue was empty. */
struct tdata *removeq() {
	pthread_mutex_lock(&qMutex);
	while (work_queue.start == 0)
		/* assert: there are no elements in work_queue */
		pthread_cond_wait(&qEmpty, &qMutex);  
	/* assert:  there is at least one element in work_queue */
	struct tdata *tmp = work_queue.start;
	work_queue.start = tmp->link;

	if (work_queue.start == 0)
		work_queue.end = &work_queue.start;

	pthread_mutex_unlock(&qMutex);
	tmp->link = 0;
	return tmp;
}

void * process_requests(void * arg)
{
	int threadNum = *((int *)arg);
	int ret;  /* return value from a call */
	while (true)
	{
		struct tdata * local_tdata = removeq();

		if (local_tdata->socket == -1)
		{
			printf("exiting thread\n");
			return NULL;
		}

		FILE *logFile;
		logFile = fopen("log.txt", "a");

		int clientd = local_tdata->socket;
		char buff[MAXBUFF];  /* message buffer */
		if ((ret = recv(clientd, buff, MAXBUFF-1, 0)) < 0) {
			perror("recv()");
			return NULL;
		} 

		buff[ret] = '\0';  // add terminating nullbyte to received array of char
		printf("Received request (%d chars):\n%s\n", ret, buff);

		FILE *iFile;
		struct name myName = {0, 0, 0, 0, -1, 0};
		read_name(&myName, buff, ret);

		pthread_mutex_lock(&lMutex);
			if ((myName.tok[0][0] == 'Q') && (myName.tok[0][1] == 'U') && (myName.tok[0][2] == 'I') && (myName.tok[0][3] == 'T'))
			{
				struct tdata quitTdata = {-1, -1, NULL, NULL};
				int j;
				for (j = 0; j < NUMTHREADS; j++)
				{
					printf("TESTING\n");
					addq(&quitTdata);
				}
				exit(0);
			}
		pthread_mutex_unlock(&lMutex);

		if (myName.tok[1][0] == '/')
		{
			memmove(myName.tok[1], myName.tok[1] + 1, strlen(myName.tok[1]));
		}

		//print request to log
		time_t now;
		now = time(NULL);
		char timestamp[30];
		if(strftime(timestamp, 30, "%a, %d %b %Y %T %Z", gmtime(&now)) == 0)
		{
			printf("error: timestamp. Log not accurate.\n");
		}
		pthread_mutex_lock(&lMutex);
			fprintf(logFile, "%d %d %s \n%d %s %s", local_tdata->request_id, threadNum, timestamp, local_tdata->request_id, myName.tok[0], myName.tok[1]);
		pthread_mutex_unlock(&lMutex);

		if (!((myName.tok[0][0] == 'G') && (myName.tok[0][1] == 'E') && (myName.tok[0][2] == 'T'))) //GET syntax check
		{
			if ((ret = send(clientd, message4, strlen(message4), 0)) < 0)
			{
				perror("send()");
				return NULL;
			}
			return NULL;
		} else {
			printf("testing\n");
			printf("opening file: %s\n", myName.tok[1]);
			iFile = fopen(myName.tok[1], "r");
			if (iFile == NULL)
			{
				perror ("Error opening file");
				if ((ret = send(clientd, message5, strlen(message5), 0)) < 0)
				{
					perror("send()");
					return NULL;
				}
			} else {
				char sendBuff[MAXBUFF];  // message buffer
				size_t bufflen = 0;  // current capacity of buff
				size_t nchars;  // number of bytes recently read
				struct stat *stats;
				stat(myName.tok[1], stats);
				int numBytes = (int)stats->st_size;
				char numBytesChar[999];

				sprintf(numBytesChar, "%d", numBytes);
				if ((ret = send(clientd, message1, strlen(message1), 0)) < 0)
				{
					perror("send()");
					return NULL;
				}
				if ((ret = send(clientd, timestamp, strlen(timestamp), 0)) < 0)
				{
					perror("send()");
					return NULL;
				}
				if ((ret = send(clientd, message2, strlen(message2), 0)) < 0)
				{
					perror("send()");
					return NULL;
				}
				if ((ret = send(clientd, numBytesChar, strlen(numBytesChar), 0)) < 0)
				{
					perror("send()");
					return NULL;
				}
				if ((ret = send(clientd, message3, strlen(message3), 0)) < 0)
				{
					perror("send()");
					return NULL;
				}
				while (fgets(sendBuff, MAXBUFF, iFile))
				{
					if ((ret = send(clientd, sendBuff, MAXBUFF-1, 0)) < 0)
					{
						perror("send()");
						return NULL;
					}
				}
			}
		}

		shutdown(clientd, 0);
		if (myName.tok[3] == NULL)
		{
			if ((ret = close(clientd)) < 0)
			{
				perror("close(clientd)");
				return NULL;
			}
		}
		fclose(logFile);
		free(local_tdata->link);
	}
}


int main(int argc, char **argv) {
	char *prog = argv[0];
	int port;
	int serverd;  /* socket descriptor for receiving new connections */
	int ret; //return value
	int request_id;
	request_id = 0;

	pthread_t tHandles[NUMTHREADS + 1];

	work_queue.start = 0;
	work_queue.end = &work_queue.start;
	pthread_mutex_init(&qMutex, NULL);
	pthread_mutex_init(&lMutex, NULL);
	pthread_cond_init(&qEmpty, NULL);
	int id[NUMTHREADS];
	int i;
	for (i = 0; i < NUMTHREADS; i++)
	{
		id[i] = i;
	}

	FILE *logFile;
	logFile = fopen("log.txt", "a");

	if (argc < 2)
	{
		printf("Usage:  %s port\n", prog);
		return 1;
	}
	port = atoi(argv[1]);

	if ((serverd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("%s ", prog);
		perror("socket()");
		return 1;
	}
	
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = INADDR_ANY;

	if (bind(serverd, (struct sockaddr*) &sa, sizeof(sa)) < 0)
	{
		printf("%s ", prog);
		perror("bind()");
		return 1;
	}
	if (listen(serverd, 5) < 0)
	{
		printf("%s ", prog);
		perror("listen()");
		return 1;
	}

	for (i = 0; i < 1; i++)
	{
		pthread_create(&tHandles[i], NULL, process_requests, (void *)&id[i]);
	}

	struct sockaddr_in ca;
	int size = sizeof(struct sockaddr);
	int clientd;

	while(true)
	{
		printf("Waiting for a incoming connection...\n");
		if ((clientd = accept(serverd, (struct sockaddr*) &ca, &size)) < 0)
		{ 
			perror("accept()");
			printf("error in accept\n");
			return NULL;
		}
		request_id++;

		struct tdata td = {clientd, request_id, logFile, NULL};

		addq(&td);
	}

	for (i = 0; i < NUMTHREADS + 1; i++)
	{
		printf("Joining thread number %d\n", i);
		pthread_join(tHandles[i], NULL);
	}

	if ((ret = close(serverd)) < 0)
	{
		printf("%s ", prog);
		perror("close(serverd)");
		return 1;
	}
	pthread_mutex_destroy(&lMutex);
	pthread_mutex_destroy(&qMutex);
	pthread_cond_destroy(&qEmpty);
	fclose(logFile);
	return 0;
}