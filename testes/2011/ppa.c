/* ppa.c
a) Escreva um programa que processe os argumentos da linha
de comando, inicialize a estrutura de tipo "vector_t" a usar e,
finalmente, mostre o estado do vector partilhado por via da invocação
da função "print_vector()" (incluída em util.c)

Exemplo de invocação:
     ./ppa 6000 4000
*/

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

vector_t vector;	// estrutura de dados a partilhar pelos threads
void * initer(void *args){
	int *p = (int *)args;
	int X = p[0];
	int Y = p[1];

	int Z = (int) &vector;

	printf ("Thread %d: cnt = %d, vp = %p\n", X, Y, Z);
	while(vector.cnt[X] < Y){
		vector.array[vector.next] = X;
		vector.next++;
		vector.cnt[X]++;
	}

	print_vector((vector_t*)Z);
	return (void*)1;
}
int main(int argc, char *argv[]) {
setbuf(stdout, NULL);




// a preencher com o seu código!
	chkr_arg_t checker_args;
	int num_one,num_zero;
	pthread_t tiniter[2],tchecker;
	int argumentos[2],args[2];

	if(argc != 3){
		print_usage(argv[0]);
		exit(-1);
	}
	parse_int(argv[2],&num_zero,2);
	parse_int(argv[1],&num_one,1);

	//INIT CHECKR_ARGS
	checker_args.argc = argc;
	checker_args.argv = argv;
	checker_args.vector = &vector;

	printf("CHCEKER ARGS argc %d  argv[1] %d argv[2] %d &vector %p \n\n\n",checker_args.argc,atoi(checker_args.argv[1]),atoi(checker_args.argv[2]),checker_args.vector);
	printf("NUM ONE %d NUM ZERO %d \n\n",num_one,num_zero);

	vector.array = (int*) malloc(sizeof(int)*(num_one+num_zero));
	vector.len = num_zero+num_one;
	vector.next = 0;
	vector.cnt[0] = 0;
	vector.cnt[1] = 0;
	//vector.lock = PTHREAD_MUTEX_INITIALIZER;
	argumentos[0] = 1;
	argumentos[1] = num_one;
	args[0] = 0;
	args[1] = num_zero;
	pthread_create(&tchecker,NULL,checker,&checker_args);
	pthread_create(&tiniter[0],NULL,initer,argumentos);
	
	pthread_create(&tiniter[1],NULL,initer,args);

	

	print_vector(&vector);

	pthread_join(tiniter[0],NULL);
	pthread_join(tiniter[1],NULL);
	
	printf ("Main thread exiting\n");
	printf("\n\nNUMS INITS %d\n\n",vector.cnt[0]+vector.cnt[1]);
return 0;
}
