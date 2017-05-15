#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>


#define BUF_SIZE 50

#define READ_SIZE 5

int main(int argc, char **argv) {

	int fd, i;
	char buffer[BUF_SIZE] = {'\0'};

	setbuf(stdout, NULL);

	if (argc != 2) {
		printf("Error in reading arguments!\n");
		exit (1);
	}

	fd = open(argv[1], O_RDWR);

	if (fd < 0) {
		printf("Error opening file descriptor!\n");
		exit (2);
	}

	while(1){
		int bytes_read = read(fd, buffer, READ_SIZE);
		printf("%d\n", bytes_read);
		//printf("Debug, %s\n", strerror(errno));
		buffer[bytes_read]='\0';
		printf("%s\n",buffer);
	}

	close(fd);

  return 0;
}
