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
  因为 writesemid readsemid 没有赋值，默认都是 0 ，所以先运行写进程，下面代码会执行
  只能对readsemid V操作，这样，唤醒读进行，不然对写进程唤醒，会丢失数据，下面代码阻塞写进程，唤醒读进程
  */
  if ((semctl(writesemid,0,GETVAL)==0)&&(semctl(readsemid,0,GETVAL)==0)) {
    buf.sem_num = 0;
    buf.sem_op = 1;
    if ((semop(readsemid,&buf,1))<0) {
      perror("avoid lock dead");
      exit(EXIT_FAILURE);
    } else {
      printf("avoid lock dead\n" );
    }
  }

  shmbuf[0] = 'a'-1;
  int temp = 0;
  while (1) { //能够走到这里，说明读进程起来了，并且可以直接读数据 writesemid 1 ，readsemid 0
    buf.sem_num = 0;// 对writesemid P操作，writesemid 为0
    buf.sem_op = -1;
    buf.sem_flg = 0;
    if ((semop(writesemid,&buf,1))<0) {
      perror("\nwake write process");
      exit(EXIT_FAILURE);
    }else{
      printf("\n wake write process\n" );
    }
    shmbuf[0] = shmbuf[0] +1;//通过共享内存映射的地址进程写数据

    buf.sem_num = 0;// readsemid readsemid 为1 写完数据，唤醒读进行读数据
    buf.sem_op = 1;
    buf.sem_flg = IPC_NOWAIT;
    if ((semop(readsemid,&buf,1))<0) {
      perror("\nwake read process");
      exit(EXIT_FAILURE);
    }else{
      printf("\nwake read process\n" );
    }
    printf("write # %d char : %c \n",temp++,shmbuf[0] );
    sleep(1);
  }
  exit(EXIT_FAILURE);
}
