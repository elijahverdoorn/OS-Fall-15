/* library source for new system calls */

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <sys/types.h>

extern int errno;

#define __NR_dub		 41
#define __NR_erv_test_call       359
#define __NR_erv_get_val         361
#define __NR_erv_put_val         360


int dub(int fd) {
  return syscall(__NR_dub, fd);
}

int test_call() {
  //printf("test");

  return syscall(__NR_erv_test_call);
}

int get_val(int arg) {
  return syscall(__NR_erv_get_val, arg);
}

int put_val(int arg) {
  return syscall(__NR_erv_put_val, arg);
}

/* The following would appear in the caller's program...

main()
{
  dub(1);
}


*/
