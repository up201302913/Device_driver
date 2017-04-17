#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[]){
	if(argc < 2){
		printf("Usage: %s device\n", *argv);
		exit(0);
	}
	char buffer[512];
	strcpy(buffer, "insmod echo.ko");
	system(buffer);
	strcpy(buffer, "rmmod echo");
	int fd = open(argv[1], O_RDWR);
	if(fd < 0){
		printf("Error opening device\n");
		perror("");
		system(buffer);
		exit(0);
	}
	strcpy(buffer, argv[1]);
	strcpy(buffer+strlen(argv[1]), " device driver test string");
	printf("Device %s opened; fd: %d\n", argv[1], fd);
	int ret = write(fd, buffer, strlen(buffer));
	printf("Wrote %d bytes:\n%s\n", ret, buffer);
	getchar();
	ret = read(fd, buffer, 512);
	printf("Read %d bytes\n%.*s\n", ret, ret, buffer);
	close(fd);
	printf("Device closed\n");
	strcpy(buffer, "rmmod echo");
	system(buffer);
	return 0;
}
