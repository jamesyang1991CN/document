#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

# define BUFSZ 4096

int main(int argc, char const *argv[]) {
  int shmid,readsemid,writesemid;
  char * shmbuf;
  struct sembuf buf;

  if ((readsemid = semget(999,1,0666|IPC_CREAT))<0) {
    perror("create read sem ");
    exit(EXIT_FAILURE);
  } else {
    printf("create read sem \n" );
  }

  if ((writesemid = semget(1099,1,0666|IPC_CREAT))<0) {
    perror("create write sem");
    exit(EXIT_FAILURE);
  } else {
    printf("create write sem \n" );
  }

  if ((shmid = shmget(888,BUFSZ,0666|IPC_CREAT))<0) {
    perror("create share memory");
    exit(EXIT_FAILURE);
  } else {
    printf("create share memory \n" );
  }

  if ((shmbuf = shmat(shmid,0,0))<(char *)0) {
    perror("get share memory");
    exit(EXIT_FAILURE);
  } else {
    printf("get share memory\n" );
  }

  /*避免死锁*/
  if ((semctl(writesemid,0,GETVAL)==0)&&(semctl(readsemid,0,GETVAL)==0)) {
    buf.sem_num = 0;
    buf.sem_op = 1;
    if ((semop(writesemid,&buf,1))<0) {
      perror("avoid lock dead");
      exit(EXIT_FAILURE);
    } else {
      printf("  avoid lock dead\n" );
    }
  }

  shmbuf[0] = 'a'-1;
  int temp = 0;
  while (1) {
    buf.sem_num = 0;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    if ((semop(readsemid,&buf,1))<0) {
      perror("wake read process");
      exit(EXIT_FAILURE);
    }else{
      printf("  wake read process\n" );
    }
    write(STDOUT_FILENO,shmbuf,1);
    sleep(3);

    buf.sem_num = 0;
    buf.sem_op = 1;
    buf.sem_flg = IPC_NOWAIT;
    if ((semop(writesemid,&buf,1))<0) {
      perror(" \n wake write process");
      exit(EXIT_FAILURE);
    }else{
      printf("\n wake write process\n" );
    }
  }
  exit(EXIT_FAILURE);
}
