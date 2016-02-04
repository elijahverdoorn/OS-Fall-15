// Elijah Verdoorn, CSCI 273, Dick Brown, St. Olaf College, Fall 2015

#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define NUMTHREADS 2

pthread_mutex_t gLock;
int val = 10;

void *Print(void *pArg)
{
	int id = getpid();
	printf("I am the PTHREAD. My PID is %d\n", id);
	pthread_mutex_lock(&gLock);
	val += 1;
	pthread_mutex_unlock(&gLock);
	return 0;
}

int main (int argc, char **argv, char **envp)
{
	pthread_t tHandles;
	int pid = getpid();
	int ret;

	pthread_mutex_init(&gLock,NULL);
	printf("I am the MAIN. My PID is %d\n", pid);
	ret = pthread_create(&tHandles, NULL, Print, (void *)5);
	if (ret)
	{
		printf("Error, value returned from create_thread: %d\n", ret);
	} else {
		printf("I am the MAIN, and I successfully launched a pthread.\n");
		pthread_mutex_lock(&gLock);
		val -= 1;
		pthread_mutex_unlock(&gLock);
		ret = pthread_join(tHandles, NULL);
		if(ret)
		{
			printf("Error on join()\n");
			exit(-1);
		}
		printf("I am the MAIN, the pthread has finished\n");
		printf("val: %d\n", val);
	}

	pthread_exit(NULL);
	return 0;
}