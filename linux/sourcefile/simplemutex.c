#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>


void task1(void);
void task2(void);
void task3(void);
int shareid=0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int main(int argc, char const *argv[]) {
  pthread_t thrd1,thrd2;
  int ret;
  ret = pthread_create(&thrd1,NULL,(void *)task1,NULL);
  ret = pthread_create(&thrd2,NULL,(void *)task2,NULL);
  pthread_join(thrd1,NULL);
  pthread_join(thrd2,NULL);
  printf("shareid = %d\n",shareid );



  return 0;
}

  void task1(void) {
    long i,tmp;
    for ( i = 0; i < 10000; i++) {
      if (pthread_mutex_lock(&mutex)!=0) {
        perror("pthread_mutex_lock");
          exit(EXIT_FAILURE);
      }
      tmp = shareid;
      tmp = tmp+1;
      shareid = tmp;
      if (pthread_mutex_unlock(&mutex)!=0) {
        perror("pthread_mutex_unlock");
        exit(EXIT_FAILURE);
      }
    }
    printf("task1 finished shareid = %d\n" ,shareid);
  }

  void task2(void) {
    long i,tmp;
    for ( i = 0; i < 5000; i++) {
      if (pthread_mutex_lock(&mutex)!=0) {
        perror("pthread_mutex_lock");
          exit(EXIT_FAILURE);
      }
      tmp = shareid;
      tmp = tmp+1;
      shareid = tmp;
      if (pthread_mutex_unlock(&mutex)!=0) {
        perror("pthread_mutex_unlock");
        exit(EXIT_FAILURE);
      }
    }
    printf("task2 finished shareid = %d\n" ,shareid);
  }
