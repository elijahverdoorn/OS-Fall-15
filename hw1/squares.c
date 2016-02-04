#include <stdio.h>
#include <stdlib.h>

int main (int argc)
{
  int numSquares;
  printf("Enter a positive integer: ");
  scanf("%d", &numSquares);
  int counter = 1;
  printf("N\t | N*N\n--------------------\n"); 
  for (counter; counter <= numSquares; counter++)
    {
      int current = counter * counter;
      printf("%i\t | %i\n", counter, current);
    }
  printf("\n");

  exit(argc);
}

  
