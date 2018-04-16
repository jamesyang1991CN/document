#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

# define MAXSIZE 10

int stack[MAXSIZE][2];
int size = 0;
sem_t sem;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void provide_data(void){

  int i;
  for (i = 0; i < MAXSIZE; i++) {
    stack[i][0] = i;
    stack[i][1] = i;
    sem_post(&sem);// 信号量 V操作 +1
  }
}


void handle_data1(void) {
  int i;
  while (pthread_mutex_lock(&mutex),((i =size++)<MAXSIZE)) {
    pthread_mutex_unlock(&mutex);
    sem_wait(&sem);
    printf("Plus : %d + %d =%d \n",stack[i][0],stack[i][1],stack[i][0]+stack[i][1] );
  }
  pthread_mutex_unlock(&mutex);
}

void handle_data2(void) {
  int i;
  while (pthread_mutex_lock(&mutex),((i =size++)<MAXSIZE)) {
    pthread_mutex_unlock(&mutex);
    sem_wait(&sem);
    printf("Multiple : %d × %d =%d \n",stack[i][0],stack[i][1],stack[i][0]*stack[i][1] );
  }
  pthread_mutex_unlock(&mutex);
}

int main(int argc, char const *argv[]) {
  pthread_t thd1,thd2,thd3;
  sem_init(&sem,0,0);
  pthread_create(&thd1,NULL,(void *)handle_data1,NULL);
  pthread_create(&thd2,NULL,(void *)handle_data2,NULL);
  pthread_create(&thd3,NULL,(void *)provide_data,NULL);

  pthread_join(thd1,NULL);
  pthread_join(thd2,NULL);
  pthread_join(thd3,NULL);
  sem_destroy(&sem);
  return 0;
}
