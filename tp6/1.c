#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "stack6.h"

#define DEBUG 0
#define MAX_THREADS 3

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c  = PTHREAD_COND_INITIALIZER;

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

struct stack *init(int size){	// initialize stack. Returns pointer to //  to initialized stack of desired size
	struct stack *st = (struct stack*) malloc(sizeof(struct stack));
	st->size = size;
	st->top = 0;	
        int i;
        
        st->sp = (struct stack_el**) malloc(sizeof(struct stack_el *)*size);
        printf("size %d\n",st->size);
	/*for(i = 0;i < size; i++){
            st->sp[i] = (struct stack_el) malloc(sizeof(struct stack_el)*size);
            
        }*/
	return st;				
}		
		
int push(struct stack *sp, struct stack_el *el){ // returns 0 if the stack is full, and 1 otherwise
    pthread_mutex_lock(&m);
    while(sp->top == sp->size){
        pthread_cond_wait(&c, &m);
    }
    
     //   return 0;
    
    sp->sp[sp->top] = el;
    sp->top++;
    el->n = sp->top;
    pthread_cond_signal(&c);
    
    pthread_mutex_unlock(&m);
    return 1;
}

struct stack_el *pop(struct stack *sp){ // returns NULL if the stack is empty
        pthread_mutex_lock(&m);
        while(sp->top == 0){
            pthread_cond_wait(&c,&m);
            
        }
            //return NULL;
        
        struct stack_el *el = sp->sp[sp->top-1];
        sp->sp[sp->top] = NULL;
        
        sp->top--;
        pthread_cond_signal(&c);
    
    pthread_mutex_unlock(&m);
        return el;
}

void show(struct stack *sp){
        int i;
        for(i=0;i<sp->top;i++){
            printf("%d elemento - %s \n",i,sp->sp[i]->str);
            
        }
}

void sfree(struct stack *sp){	// frees all data structures allocated
        free(sp->sp);
        free(sp);
    
}

int main(int argc,char *argv[]){
	/*
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
	}*/

	//printf("counter - %ld ---- %p \n",variavel,&variavel);
	struct stack *oi = init(100);
        struct stack_el *el = (struct stack_el*) malloc(sizeof(struct stack_el));
        struct stack_el *el2 = (struct stack_el*) malloc(sizeof(struct stack_el));
        struct stack_el *el3;
        
        printf("size 2    - - - %d\n\n",(int)sizeof(oi));       
        el->str = "Abajgtdcaxi \n";
        push(oi,el); 
         el2->str = "1";
        push(oi,el2); 
        el3 = pop(oi);
        
        if(el3 == NULL)
            return 111;
        
    //    show(oi);
        
        printf("\n\nfrase %s\n\n",el3->str);
     
    
        
	return 1;
}
