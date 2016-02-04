/* test program for lib.c */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "lib.h"

int trydub()
{
  int rv;
  char *msg = "Hello, world!\n";
  printf("dub(1) returns %d\n", rv = dub(1));

  printf("Test of write on fd %d:\n", rv);
  write(rv, msg, strlen(msg));

  return 0;
}

int trytest()
{
  printf("This is from GETPID: %d\n", getpid());
  printf("This is from TESTCALL: %d\n", test_call());
  return 0;
}

int tryput(int arg)
{
  printf("trying the put call with value %d\n", arg);
  
  put_val(arg);
}

int tryget(int arg)
{
   printf("trying the get call\n");
   printf("getval returned %d\n", get_val(arg));
   
}

int main()
{
  tryput(1995);
  tryget(getpid());
}
