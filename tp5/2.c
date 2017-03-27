#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define DEBUG 0
#define MAX_THREADS 3


typedef struct {
    long *cnt;		/* pointer to shared counter */
    int n;		/* no of times to increment */
    int id;		/* application-specific thread-id */
} targ_t;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *tfun(void *arg) {

    targ_t *targ = (targ_t *) arg;
    int i;
	int aux;
  	
    printf("Thread %d starting\n", targ->id);
    printf("tager %d \n", targ->n);
    printf ("&cnt = %p \n", targ->cnt);

    for(i = 0; i < targ->n ; i++) {
		if( DEBUG ) 
		    printf("b4:  %ld ", *(targ->cnt)); 
		
		/*pthread_mutex_lock(&lock);
		(*(targ->cnt))++; 
		pthread_mutex_unlock(&lock);*/
		/*If cnt is always N*MAX_ITER 
		   Then comment the previous line and 
		   uncomment the following 3 lines */
		pthread_mutex_lock(&lock);
		   aux = *(targ->cnt);
		   aux++;
		   *(targ->cnt) = aux; 
		pthread_mutex_unlock(&lock);

		if (DEBUG )
		    printf("\t %ld \n", *(targ->cnt)); 
		//printf("variavel - %d \n\n",targ->cnt);
    }
    printf("Thread %d done\n", targ->id);
    return NULL;
}


int main(int argc,char *argv[]){
	
	int NUM = atoi(argv[1]);	
  	pthread_t ps[MAX_THREADS];
	int i=0;
	long variavel = 0;
	targ_t counter[MAX_THREADS];
	
	
	for(i=0;i<MAX_THREADS;i++){
		counter[i].n = NUM;
		counter[i].id = i;
		counter[i].cnt = &variavel;
		pthread_create(&ps[i],NULL,tfun,&counter[i]);
	}

	for(i=0;i<MAX_THREADS;i++){
		pthread_join(ps[i],NULL);
	}

	printf("counter - %ld ---- %p \n",variavel,&variavel);
	return 1;
}
