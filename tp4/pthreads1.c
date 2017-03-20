#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct{
  int a;
  int b;
}operands;

void *sum(void *args){
  int *vec = (int *)args;
  int c = *vec + *(vec+1);
//  int a = vec[2];
  //printf("%d + %d = %d \n",*vec,*(vec+1),c);
  //(*a) = c;
  int *r = (int *)malloc(sizeof(int));
  *r = c;
  return (void*)(r);
  //pthread_exit(c);
}

void *diff(void *args){
  int *vec = (int *)args;
  int c = *vec - *(vec+1);

  int *r = (int *)malloc(sizeof(int));
  *r = c;
  return (void*)(r);
}

void *mult(void *args){
  operands *vec = (operands *)args;

  int c = vec->a * vec->b;


  int *r = (int *)malloc(sizeof(int));
  *r = c;
  return (void*)(r);
}

void *divi(void *args){
  operands *vec = (operands *)args;

  int c = vec->a / vec->b;


  int *r = (int *)malloc(sizeof(int));
  *r = c;
  return (void*)(r);
}

int main(int argc,char *argv[]){
  int a[3];
  operands vec;
  int **res;


  a[0] = atoi(argv[1]);
  a[1] = atoi(argv[2]);
  //a[2] = &res;

  vec.a = a[0];
  vec.b = a[1];

  printf("%d %d\n\n",a[0],a[1]);
  pthread_t p1,p2,p3,p4;

  pthread_create(&p1,NULL,sum,a);
  pthread_join(p1,(void **)&res);
  printf("%d + %d = %d\n",a[0],a[1],*res);

  pthread_create(&p2,NULL,diff,a);
  pthread_join(p2,(void **)&res);
  printf("%d - %d = %d\n",a[0],a[1],*res);

  pthread_create(&p3,NULL,mult,a);
  pthread_join(p3,(void **)&res);
  printf("%d * %d = %d\n",a[0],a[1],*res);

  pthread_create(&p4,NULL,divi,a);
  pthread_join(p4,(void **)&res);
  printf("%d / %d = %d\n",a[0],a[1],*res);




  return 0;
}
