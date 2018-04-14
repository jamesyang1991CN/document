

**直接测试**

![shmwr](https://github.com/jamesyang1991CN/document/blob/master/picture/shmwr2.png)

![shmrd](https://github.com/jamesyang1991CN/document/blob/master/picture/shmrd2.png)


上面的测试案例，告诉我们如果没有同步，由于发送端和读取端因为读取间隔不一样造成数据丢失，模拟了系统繁忙情况，所以同步很重要信号量作用很大，保证了数据同步


## 信号量
初次看信号量蛮难理解，但其实也很简单

**信号量实现原理**
 linux 中 有个专门的机制操作信号量  被称为PV操作，用来进行同步和互斥
 简单来说就是给一个变量sem（初始值为1） 设置值 0/1 当P操作就是-1 使sem从1变成0 进行V操作就是+1 使sem 从0变为1  当sem 为1 相应的唤醒进程 反之阻塞进程

 1. PV操作都是原子操作
 2. p操作代表进程占用一个资源，v操作代表进程释放一个资源

**使用信号量同步共享内存访问的设计**
共享内存进程间同步 设置了两个变量 readsem (用于控制读 初始值为0)和writesem(用于控制写 初始值是1)

![shmrd](https://github.com/jamesyang1991CN/document/blob/master/picture/signal_ipc.png)
 上面的模型 用两个变量writesem 和 readsem 分别控制写读进程，当writesem 变为1 ，唤醒写进程，写进程完成 readsem 变为0，；当readsem 变为1，唤醒读进程，读进程完成将writesem 变为0

不管对readsem 还是 writesem 操作，都是通过PV操作 (可能这里还不理解，下面的例子能说明)

这里看代码逻辑

写进程
```c
while (1) {
  P(writesem);
  // 因为writesem初始值1 唤醒写进程，P操作writesem变为0，此时 readsem初始为0所以阻塞读进程
  write share memory //开始操作
  V(readsem);
  // 写进程完成后，通过v操作，readsem 变成1，唤醒读进程 ，此时writesem 为0 阻塞写进程再次写数据
}

```

读进程
```c
while(1){
  P(readsem);
  //readsem 初始值0 所以 写进程一开始就被阻塞了，如果readsem 在写进程完成后，变为1 ，开始读进程
  //此时writesem 为0 阻塞写进程
  read share memory
  V(writesem);
  //读完后，V操作 writesem 变为1 唤醒写进程 此时读进程为0 阻塞读进程
}

```

如果上面的解释还是不理解，只能从代码执行的步骤分析了
条件 readsem 初始值 0 ，writesem 初始值 1
1.先运行 读进程， 但是readsem 初始值是0 ，所以读进程一开始就被阻塞了，但是writesem 初始值是1 所以唤醒 写进程
2. 写进程运行， 对writesem 进行P操作 变成0，此时开始对 share memory 进程write 数据
3. 操作完成后，对readsem 进程V操作，将值变成1 ，唤醒读进程，此时writesem进程为0 阻塞读进程
4. 此时readsem为1，先对它进程P操作，把readsem变成0 ，读进程运行，read share memory
5. 操作完成后，对writesem 进行V操作，变成1 ，所以唤醒读进程，但是readsem还是为0 ，所以读进程被阻塞了
...

以上是信号量同步互斥的原理

**函数介绍**

创建信号量
```c
int simget(key_t __key,int __nsems,int __semflg);
/*
功能 创建信号量
__key 标识信号量
__nsems 信号量个数 一般是1，严格来说 在一个信号量组中的信号量个数
__semflg 信号量访问权限
成功返回去信号量标识码 __semid 失败-1
*/
```
控制信号量信息
```c
int semctl(int __semid,int __semnum,int __cmd,...);
/*
__semid 信号量标识码
__semnum 信号量编号，信号量组的编号
__cmd 进行的操作
IPC_START 读取信号量集 的数据结构 semid_ds 并存储在union semnum 第四个参数buf中
IPC_SET 设置数据结构 semid_ds 用union semnum 第四个参数buf 数据
IPC_RMID 信号量集 从内存中删除
GETALL 读取信号量集中的所有信号量的值
GETNCNT 返回正在等待资源的进程数目
GETPID 返回最后一个执行semop 操作的进程PID
GETVAL 返回信号量集中翻个信号量值
GETZCNT 返回正在等待完全空闲的资源进程数目
SETALL 设置信号量集中所有信号量的值
SETVAL 设置信号量集中一个信号的值，在union semnum 类型第四个参数 val中
*/
//最常用 SETVAL GETVAL
```
```c
union semnun{
  int val；// value for setval
  struct semid_ds *buf; //buffer for ipc_start ipc_set
  unsigned short *array; //array for GETALL SETALL
  struct seminfo * __buf;//buffer for ipc_info

}
```

对信号箱进程PV操作
```c
int semop(int __semid,struct sembuf * __sops,sieze_t __nsops);
/*
__semid PV执行的信号量组
__sops 结构体指针，包含具体PV操作
*/
struct sembuf{
  unsigned short sem_num;
  short sem_op;
  short sem_flg;
}
/*
semop_num 操作信号在信号集中的编号 第一个为0
sem_op P操作为-1 V操作+1
sem_flg 操作标志 、
IPC_NOWAIT 信号的操作不能满足直接返回 设定错误信息
C_UNDO 程序结束，信号的值恢复到操作前的值
*/
```

**编程步骤**

写进程
1. 创建writesemid 和readsemid 两个信号量变量
2. 创建共享内存
3. 将共享内存映射到写进程的地址
4. 对writesemid 进程P操作，开始写数据
5. 写完数据对readsemid 进行V操作

读进程
1. 创建writesemid 和readsemid 两个信号量变量
2. 创建共享内存
3. 将共享内存映射到读进程的地址
4. 对 readsemid 进程P操作，开始写数据
5. 写完数据对writesemid 进行V操作


**代码debug**
写进程
```c
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
    if ((semop(readsemid,&buf,1))<0) {
      perror("avoid lock dead");
      exit(EXIT_FAILURE);
    } else {
      printf("avoid lock dead\n" );
    }
  }

  shmbuf[0] = 'a'-1;
  int temp = 0;
  while (1) {
    buf.sem_num = 0;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    if ((semop(writesemid,&buf,1))<0) {
      perror("\nwake write process");
      exit(EXIT_FAILURE);
    }else{
      printf("\n wake write process\n" );
    }
    shmbuf[0] = shmbuf[0] +1;

    buf.sem_num = 0;
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
```
如上，第一次运行当因为readsemid,writesemid没有值，所以设置避免死锁的方式，读进程信号量变为1写进程信号量为0 ，这样写进程阻塞，等待读进程起来，此时 写进程一直阻塞，打印avoid lock dead ，直到启动读进程改变 writesemid的值，唤醒写进程，可以看下面读进程代码

读进程
```c
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

```
第一次运行，虽然读进程也用避免死锁的方式，但是此时 writesemid 为0，readsemid为1（因为写进程已经PV操作过了），读进程开始读取数据，分别对writesemid 和readsemid 进行PV操作，最后，writesemid为1 readsemid 为0 唤醒写进程，阻塞读进程，写进程开始写数据

可以运行debug，代码就很清楚了

[带有注释版本代码][3]

**测试debug**
测试结果，肯定没有丢失数据
![writesem](https://github.com/jamesyang1991CN/document/blob/master/picture/writesem.png)

![readsem](https://github.com/jamesyang1991CN/document/blob/master/picture/radsem.png)


```c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
int msgget(key_t key,int flag);
/*
创建或者获取消息对象成功，返回消息队列的id
*/
```

```c
int msgsnd(int msgid,const void * ptr,size_t nbytes,int flag);
/*
msgid 消息队列的id
ptr 指向结构体，长整型 类似下面的结构体
*/
struct mymsg{
  long mtype; /*消息类型*/
  char mtext[512]; /*字符型消息*/
}

```

```c
int msgrcv(int msgid,void *ptr,size_t nbyte,long type,int flag);
/*
成功获取消息 返回数据长度
type 表示选择获取消息的类型，
type == 0 返回队列中的第一个消息
type > 0  返回队列消息类型为type的第一个消息
type<0 返回队列消息中小鱼或者等于type绝对值，是最小的那一个
type非0 就说明消息队列获取方式是非FIFO方式
*/
```










---
[1]:https://blog.csdn.net/engineer_james/article/details/79897855
[2]:https://blog.csdn.net/engineer_james/article/details/79902399
[3]:https://download.csdn.net/download/engineer_james/10348756
