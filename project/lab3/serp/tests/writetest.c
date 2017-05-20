#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 150

int main(int argc, char **argv) {

  int fd, i;
  char buffer[BUF_SIZE];

  setbuf(stdout, NULL);

  if (argc != 2) {
    printf("Error in reading arguments!\n");
    exit (1);
  }

  fd = open(argv[1], O_RDWR);

  if (fd < 0) {
    printf("Error opening file discriptor!\n");
    exit (2);
  }
  buffer[0]='\n';
  scanf("%99[^\n]", &buffer[1]);
  write(fd,buffer,strlen(buffer));
  close(fd);

  return 0;
}
