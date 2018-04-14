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
  //创建控制读进程的信号量 readsemid
  if ((readsemid = semget(999,1,0666|IPC_CREAT))<0) {
    perror("create read sem ");
    exit(EXIT_FAILURE);
  } else {
    printf("create read sem \n" );
  }
  //创建控制写进程的信号量 writesemid
  if ((writesemid = semget(1099,1,0666|IPC_CREAT))<0) {
    perror("create write sem");
    exit(EXIT_FAILURE);
  } else {
    printf("create write sem \n" );
  }
  //创建共享内存 获取共享内存的id
  if ((shmid = shmget(888,BUFSZ,0666|IPC_CREAT))<0) {
    perror("create share memory");
    exit(EXIT_FAILURE);
  } else {
    printf("create share memory \n" );
  }
  //将共享内存映射到写进程 获取内存在本进程的地址shmbuf ，其实通过字符数组形式直接写数据
  if ((shmbuf = shmat(shmid,0,0))<(char *)0) {
    perror("get share memory");
    exit(EXIT_FAILURE);
  } else {
    printf("get share memory\n" );
  }

  /*避免死锁*/
  /*
  因为这里我先运行写进程，writesemid 为0 readsemid 为1，多以这里避免思索的代码不会走，
  直接进行下面的读操作
  但是先走读进程，那么读进程会阻塞在avoid lock dead 直到唤醒写进程
  */
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
  //下面的就是读进行的设计模型的代码，读进程读数据
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
