#include <stdio.h>
#include <ctype.h> // for isspace()
#include <stdlib.h>
#include <string.h> // for strcpy();

#define MAXTOKS 100

// TODO: this needs work! read on enums
enum status_value {NORMAL, EOF_OR_ERROR, TOO_MANY_TOKENS};

struct name
{
	char** tok; //hold "tokens" AKA strings
	int count; //hold number of words
	int status; //hold info about status of parsing
};

int read_name(struct name *arg)
{
	arg->tok = malloc(sizeof(char *) * MAXTOKS);
	char* getlineBuff = NULL;
	size_t getlineLen = NULL;

	arg->status = NORMAL;

	int numCharsInLine = getline(&getlineBuff, &getlineLen, stdin);

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
			numCharsInTok++;
		}

		arg->tok[arg->count] = malloc((sizeof(char) * numCharsInTok) + 1);
		int i = 0; // counting variable
		int j = 0;
		char temp[numCharsInTok + 1];
		for (j; j <= numCharsInTok; j++)
		{
			temp[j] = getlineBuff[startingPlaceInBuff + j];
		}
		temp[numCharsInTok + 1] = '\0';
		strcpy(arg->tok[arg->count], temp);

		//save data and prep to reloop
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
			char temp[numCharsExtra + 1];
			for (j; j <= numCharsExtra; j++)
			{
				temp[j] = getlineBuff[startingPlaceInBuff + j];
			}
			temp[numCharsExtra + 1] = '\0';
			strcpy(arg->tok[arg->count], temp);
			arg->status = TOO_MANY_TOKENS;
			break;
		}
	}
	if (arg->status == NORMAL)
	{
		return 1;
	} else {
		return 0;		
	}
}

int main(int argc, char const *argv[])
{
	struct name myName = {0, 0, 0};
	read_name(&myName);

	printf("status %s\n", myName.status);

	int i = 0;
	for (i; i <= myName.count; i++)
	{
		printf("%s\n",myName.tok[i]);
	}
	
	

	return 0;
}