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


int main()
{
   FILE *iFile;
   FILE *oFile;
   char *buffer = NULL;
   size_t bufflen = 0; 
   struct name myName = {0, 0, 0, 0, -1, 0};
   printf("Enter a filename to be copied from, then a file to be copied to.\n");
   read_name(&myName);
   iFile = fopen(myName.tok[0], "r");
   oFile = fopen(myName.tok[1], "w");
   if (iFile == NULL || oFile == NULL) 
     perror ("Error opening file");
   else {
     while (getline(&buffer, &bufflen, iFile) >= 0)
       fprintf(oFile, "%s", buffer);
     fclose(iFile);
     fclose(oFile);
   }
   return 0;
}