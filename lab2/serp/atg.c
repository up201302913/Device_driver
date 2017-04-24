#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>



int main(int argc, char *argv[],char *envp[]){
	int i=0;
	char *frase = "BOM DIA COMO VAI A SUA TIA!\n";
	char *frase2;// = (char *) malloc(sizeof(char)*(strlen(frase)+1));

	printf("TAMANHO FRSE %d\n",strlen(frase));

	int fd = open("/dev/echo",O_RDWR,S_IRWXU);

	write(fd,frase,strlen(frase));
	printf("BOns dias");
	read(fd,frase2,strlen(frase)+1);
	//while(1);
	getchar();
	return 1;
}
