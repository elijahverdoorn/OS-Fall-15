#include <stdio.h>
#include <stdlib.h>

int main (int argc, char **argv, char **envp)
{
  int i = argc;
  while (i--)
    {
      printf("%s ", argv[i]);
    }
  printf("\n");

  exit(argc);
}

  
