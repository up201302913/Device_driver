/* UserControl test for DD! 2017
Use sudo to override security setting */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Since the Virtual Machine don't allow more than one user, the user Control
test in DD doesn't allow the same user to open more than one proccess of the DD */

int main(int argc, char **argv) {

  int fd, fd2;

  printf("User number 1 trying to open device driver:\n");

  fd = open("/dev/serp", O_RDWR, 00400);

  if(fd == -1) {
    printf("Syscall open() from user n1 failed with error : [%s].\n",strerror(errno));
  } else {
    printf("User n1 successfully opened DD!\n");
  }

  printf("User number 2 trying to open device driver:\n");

  fd2 = open("/dev/serp", O_RDWR, 00400);

  if(fd2 == -1) {
    printf("Syscall open() from user n2 failed with error : [%s].\n",strerror(errno));
  } else {
    printf("User n2 successfully opened DD!\n");
  }
  
  return 0;
}
