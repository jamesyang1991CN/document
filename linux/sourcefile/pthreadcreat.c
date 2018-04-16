#include <pthread.h>
#include <stdlib.h>
#include<string.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>

void task1(int *count);
void task2(int *count);
void cleanup(int cnt1,int cnt2);

int g1 = 0;
int g2 = 0;

int main(int argc, char const *argv[]) {

  pthread_t thd1,thd2;
  int ret;
  void *retval;

  ret = pthread_create(&thd1,NULL,(void*)task1,(void*)&g1);
  ret = pthread_create(&thd2,NULL,(void*)task2,(void*)&g2);
  cleanup(g1,g2);//show cnt
  getchar();
  int temp;
  temp = pthread_join(thd1,&retval);
  if (temp != 0) {
    printf("error thd1 return\n" );
  }
  printf("return val of task1 is %d\n",(int)retval);
  cleanup(g1,g2);
  exit(EXIT_SUCCESS);

  return 0;
}

void task1(int *cnt){
  while (*cnt <5) {
    printf("task1 count：%d\n", *cnt);
    (*cnt)++;
    sleep(1);
  }
  pthread_exit((void *)100);

}

void task2(int *cnt) {
  while (*cnt<5) {
    printf("task2 count :%d\n",*cnt );
    (*cnt)++;

  }
}

void cleanup(int cnt1,int cnt2) {
  printf("total iterations: %d\n",cnt1+cnt2 );
}
