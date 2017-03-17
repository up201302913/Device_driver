#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>



int main(int argc, char *argv[],char *envp[]){
	int i=0;

	int fd = open("/dev/echo",O_RDONLY,NULL);

	while(1);
}
