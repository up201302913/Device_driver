#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define DEBUG 0
#define MAX_THREADS 3

struct stack_el{
		char *str;              /* String to be printed in show() */
		int n;                  /* Integer to be printed in show() */
		struct stack_el *next;  /* Link to next stack element */
};

void pop();
void push();
void show();

void show(void *args){
	struct stack_el *stack = (struct stack_el) *args
	struct stack_el *proximo;

	while(proximo->next != NULL){
		printf("%d elemento - %s \n",proximo->n,proximo->str);
		*proximo = proximo->next;
	}	

	return;
}

void pop(){
	struct stack_el *stack = (struct stack_el) *args
	struct stack_el *proximo;

	*proximo = stack->next;
	free(stack);
	*stack = *proximo 

	return (void*)(stack->str);
}
