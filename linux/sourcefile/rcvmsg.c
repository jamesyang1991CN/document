#进程间通信
linux 系统中有很多进程，免不了进程间进行通信，即IPC通信，linux 中有6种方式
信号、无名管道(pipe)和有名管道(FIFO)、共享内存、信号量、消息队列、套接字(socket)

socket 已经在前面研究tcp/udp的时候学习过[socket tcp][1]、[socket udp][2]

## 信号
信号是软件中断产生，用于进程间异步传递信息
一般在shell 中操作，进程获取信号进行处理，一共有64中信号，在shell中输入 **kill -l** 可查阅

![signal](https://img-blog.csdn.net/20180414115802455?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2VuZ2luZWVyX2phbWVz/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)


从来没有在这方面用过，主要的方法：

```c
#include <signal.h>
int signal(int signum,void (* handler(int)));
/*
signum kill -l 列出的信号

handler 表示 接收到信号处理函数
 返回处理成功的状态
*/
```


## 无名管道和有名管道

### 无名管道
首先认识下管道模型

![pipe1](https://img-blog.csdn.net/2018041411583069?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2VuZ2luZWVyX2phbWVz/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

创建上面的管道只需要使用pipe函数
```c
#include <unistd.h>
int pipe(int fd[2]);
/*
fd[2]数组 fd[0]读取管道   fd[1]写入管道
返回 0 表示成功 返回-1 表示失败
*/
```
上面的管道，进程自己写自己读取管道内容，并不是进程间通信。

正确的用法，因为是无名管道，只能用于父子进程之间使用，通过fork 产生子进程，然后实现父子进程通信

下面是父子进程 pipe 通信的模型

**双工通信管道**

![piped](https://img-blog.csdn.net/20180414120006459?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2VuZ2luZWVyX2phbWVz/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)
**单工通信管道**

![pipes](https://img-blog.csdn.net/20180414120027847?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2VuZ2luZWVyX2phbWVz/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)


**单工代码编码步骤：**
1. 父进程中fork 子进程
2. 用pipe函数创建管道
3. 父进程中写 所以close fd[0] 调用write 方法
4. 子进程中读信息，所有close fd[1] 调用read方法

单工代码如下：
```c
int main(int argc, char const *argv[]) {

  int pid,fd[2],n;
  char buf[BUFSIZE];
  if (pipe(fd) < 0 ) {
    printf("pipe error \n");
    exit(1);
  }
  if ((pid = fork()) < 0 ) {
    printf("fork failure\n");
    exit(1);
  }else if (pid == 0) { //child
    close(fd[1]);//关闭管道 写端
    while ((n = read(fd[0],buf,BUFSIZE)) >0 ) {//读取 子进程读端数据
      write(STDOUT_FILENO,"receive from parent: ",21);
      write(STDOUT_FILENO,buf,n);
    }
    if (n < 0 ) {
      printf("read error\n");
      exit(1);
    }
    close(fd[0]);
    printf("exit child\n");
    exit(0);
  }
  close(fd[0]);
  while ((n = read(STDIN_FILENO,buf,BUFSIZE)) >0 )
    write(fd[1],buf,n);
  if (n < 0 ) {
    printf("write error\n");
    exit(1);
  }
  close(fd[1]);
  printf("exit parent\n");
  return 0;
}

```
上面的代码 就是通过fork出子进程，然后通过pipe操作，从shell写入和写出
> 特别说明: <br/>
> STDOUT_FILENO 表示写出shell界面<br/>
> STDIN_FILENO表示从shell界面读取

### 有名管道
有名管道 比较有意思，我这边自己谢了一个demo测试了下，从名字就可以知道通过唯一的名称或者key 标识同一个管道，实现不同进程之间的通信


**编写代码的步骤**

写进程
1. open函数打开有名管道 假设是fifo
2. 使用write 将带有数据的buffer 写入管道fifo

读进程
1. 创建管道 mkfifo函数 名称为fifo
2. 打开管道 open 函数
3. 读取管道里的数据 read 读取fifo 数据buffer 打印出来

**以上注意**：首先运行 读进程 然后是写进程，会在当前目录下创建fifo文件就是管道，跑完一次记得删除，不然那第二次测试会报错

**写进程 wrfifo**
```c
int main(int argc, char const *argv[]) {

  int fd;
  int len;
  char buf[PIPEBUF];
  time_t tp;
  printf("i am %d\n",getpid() );
  if ((fd=open("fifo1",O_WRONLY))<0) {
    perror("open");
    exit(EXIT_FAILURE);
  }
  printf("between open & write\n");
  while (1) {
    /* code */
    time(&tp);
    len = sprintf(buf,"wrfifo %d sends %s ",getpid(),ctime(&tp));
    if ((write(fd,buf,len+1))<0) {
      perror("write");
      close(fd);
      exit(EXIT_FAILURE);
    }
    sleep(3);
  }
  close(fd);
  exit(EXIT_SUCCESS);
}
```
以上代码每隔3秒发送一个数据wrfifo %d sends %s ，就是然进程沉睡3秒，再执行

**读进程 rdfifo**

```c
int main(int argc, char const *argv[]) {

  int fd,fd1;
  int len;
  char buf[PIPEBUF] = "abc\n";
  mode_t mode = 0777;
  if (mkfifo("fifo1",mode) < 0) {
    printf("mkfifo error\n");
    exit(1);
  }
  if ((fd = open("fifo1",O_RDWR))< 0) {
    printf("open error %d\n",fd);
    exit(1);
  }

  while ((len = read(fd,buf,PIPEBUF-1))>0) {
    printf("rdfifo read %s\n",buf );
  }
  /*
    参数fd所指的文件传送nbyte个字节到buf指针所指的内存中。
    若参数nbyte为0，则read()不会有作用并返回0。
    返回值为实际读取到的字节数，如果返回0，
    表示已到达文件尾或无可读取的数据。错误返回-1,
    并将根据不同的错误原因适当的设置错误码。
  */
  printf("%d\n", len);
  close(fd);
  if (len == 0) {
    printf("read end\n");
  }
  exit(EXIT_SUCCESS);
}

```
上面代码实现 读取写进程每隔3秒发送的数据

**实际测试**

当读进程运行，首先创建fifo 然后等待写进程写数据，然后，写进程写数据了，每隔三秒 读进程会收到管道中的数据，然后打印出来

![rdfifo](https://img-blog.csdn.net/20180414120129216?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2VuZ2luZWVyX2phbWVz/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)
![wrfifo](https://img-blog.csdn.net/20180414120145700?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2VuZ2luZWVyX2phbWVz/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

## 共享内存 (敲黑板，划重点)
上面的通信需要经过内核，浪费系统资源，而共享内存就比较厉害，直接在用户态创建物理内存，不需要经过内核，支持大量数据传输

**共享内存的原理**：
在系统中取一块未使用的物理内存，然后两个进程独立的用户空间印射到这个物理内存，对应不同进程得到的是不同的内存地址，实现了两个进程对同一个物理内存读和写操作

因为不会涉及用户空间和内核空间的切换，所以效率高

![sharemomery](https://img-blog.csdn.net/2018041412023884?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2VuZ2luZWVyX2phbWVz/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

**函数原型**

 创建共享内存

```c
#include <sys/shm.h>
int shmget(key_t __key,size_t, __size,int _-shmflg);
/*
功能： 创建内存或者获取已创建的共享内存
__key 标识共享内存
__size 共享内存的长度
__shmflg 标志  IPC_CREATE 共享内存在需要创建，反之获取共享内存
*/

```
映射共享内存

```c
#include <sys/shm.h>
void * shmat(int shmid ,const void *shmaddr,int shmflg);
/*
功能: 映射共享内存
shmid 共享内存的id
shmaddr 共享内存在本进程中的地址，一般填0 ，函数会返回内存地址
shmflg 内存的操作模式，SHM_RDONLY 表示只读
返回共享内存虚拟起始地址 可以直接进行读写操作
*/
```

解除共享地址映射

```c
#include <sys/shm.h>
int shmdt(const void * shmaddr);
/*
功能：断开共享内存的映射
shmaddr 共享内存的本进程中的虚拟地址
返回0 成功 反之-1 失败
*/
```

删除共享内存

```c
#include <sys/shm.h>
int shmctl(int __shmid,int __cmd,struct shmid_ds *buf);
/*
功能： 共享内存控制函数
__shmid 共享内存的id
__cmd 共享内存的操作
buf 保存内存模式状态和访问权限的数据结构
成功返回0 失败返回-1
*/
```

**开发步骤**

写进程
1. shmget创建一个共享内存
2. shmat 将共享内存映射到本进程获取本进程读进程的虚拟地址
3. 根据获取共享内存的虚拟地址进行写数据

读进程
1. shmget创建一个共享内存
2. shmat 将共享内存映射到本进程获取读进程的虚拟地址
3. write 将获取的虚拟地址获取数据 打印出来

**案例代码**

写进程
```c
int main(int argc, char const *argv[]) {

    int shmid;
    char * shmbuf;//共享内存的虚拟地址起始地址

    if ((shmid = shmget(888,BUFSZ,0666|IPC_CREAT) ) < 0) {
      perror("shmget1");
      exit(EXIT_FAILURE);
    }

    printf("shmid %d\n", shmid);

    if ((shmbuf = shmat(shmid,0,0))< (char *)0) {
      perror("shmat1");
      exit(EXIT_FAILURE);
    }
    printf("shmbuf %s\n", shmbuf);

     shmbuf[0] = 'd';

   exit(EXIT_SUCCESS);
}

```

上面写进程 通过shmbuf 代表数组首地址，通过对字符数组赋值写入共享内存


读进程
```c
int main(int argc, char const *argv[]) {

  int shmid;
  char * shmbuf;


  if ((shmid = shmget(888,BUFSZ,0666|IPC_CREAT))<0) {
    perror("shmget read");
    exit(EXIT_FAILURE);
  }
  printf("shmid %d\n", shmid);
  if ((shmbuf = shmat(shmid,0,0))< (char *)0) {
    perror("shmat read");
    exit(EXIT_FAILURE);
  }
  printf("shmbuf %s\n", shmbuf);
  write(STDOUT_FILENO,shmbuf,1);
  write(STDOUT_FILENO,"\n",1);
  exit(EXIT_SUCCESS);
}

```

读进程通过获取shmbuf 共享内存首地址，显示在界面上

**测试显示**

![shmwr](https://img-blog.csdn.net/20180414120300888?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2VuZ2luZWVyX2phbWVz/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

![shmrd](https://img-blog.csdn.net/20180414120319822?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2VuZ2luZWVyX2phbWVz/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)


但是上面的代码会有问题，当出现阻塞会丢失数据

下面这个例子一下子就看出问题了，在大量数据读写的时候，会丢失数据

**读进程**
```c
int main(int argc, char const *argv[]) {

  int shmid;
  char * shmbuf;
  if ((shmid = shmget(888,BUFSZ,0666|IPC_CREAT))<0) {
    perror("shmget read");
    exit(EXIT_FAILURE);
  }
  printf("shmid %d\n", shmid);
  if ((shmbuf = shmat(shmid,0,0))< (char *)0) {
    perror("shmat read");
    exit(EXIT_FAILURE);
  }
  printf("shmbuf %s\n", shmbuf);
  while(1){
    printf("in while\n");
    write(STDOUT_FILENO,shmbuf,1);
    write(STDOUT_FILENO,"\n",1);
    sleep(3);
  }

  exit(EXIT_SUCCESS);
}
```
**写进程**
```c
int main(int argc, char const *argv[]) {
    int shmid;
    char * shmbuf;//共享内存的虚拟地址起始地址
    if ((shmid = shmget(888,BUFSZ,0666|IPC_CREAT) ) < 0) {
      perror("shmget1");
      exit(EXIT_FAILURE);
    }
    printf("shmid %d\n", shmid);
    if ((shmbuf = shmat(shmid,0,0))< (char *)0) {
      perror("shmat1");
      exit(EXIT_FAILURE);
    }
    printf("shmbuf %s\n", shmbuf);
   shmbuf[0] = 'a'-1;
   int temp = 0;
    while(1){
      shmbuf[0] = shmbuf[0]+1;
      printf("write # %d char : %c \n ",temp++,shmbuf[0]);
      sleep(1);
    }
   exit(EXIT_SUCCESS);
}

```

可以看出来这里的代码就是在写进程中变成循环 打印a b c d ..26个英文字母，频率是每隔1秒

而读进程 每隔3秒读取数据 显示，所以会有问题

**直接测试**

![shmwr](https://img-blog.csdn.net/20180414152437176?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2VuZ2luZWVyX2phbWVz/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

![shmrd](https://img-blog.csdn.net/20180414152450952?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2VuZ2luZWVyX2phbWVz/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)


上面的测试案例，告诉我们如果没有同步，由于发送端和读取端因为读取间隔不一样造成数据丢失，模拟了系统繁忙情况，所以同步很重要信号量作用很大，保证了数据同步



## 信号量
初次看信号量蛮难理解，但其实也很简单
信号量作用是为了保证 共享内存读取同步

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
写进程
![writesem](https://img-blog.csdn.net/20180414152608825?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2VuZ2luZWVyX2phbWVz/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)
读进程
![readsem](https://img-blog.csdn.net/20180414152624363?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2VuZ2luZWVyX2phbWVz/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

## linux ipc 消息队列

消息队列，有点类似，android中的四大组件之一的 广播 和广播接受者，比较简单

首先看模型
![msg](https://img-blog.csdn.net/20180414153718847?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2VuZ2luZWVyX2phbWVz/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

**函数原型**
 创建消息队列

```c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
int msgget(key_t key,int flag);
/*
创建或者获取消息对象成功，返回消息队列的id
*/
```
发送消息到消息队列
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
获取消息到消息队列
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

**实际代码测试**
发送消息
```c
int main(int argc, char const *argv[]) {

  int msgid,length;
  struct mymsg{
    long mtype;
    char mdata[512];
  }msg;

  if ((msgid = msgget(3099,0666|IPC_CREAT))<0) {
    perror("create msg quenue");
    exit(1);
  } else {
    printf("create msg quenue\n" );
  }

  msg.mtype = 1 ;
  strcpy(msg.mdata,"hello james\n");

  if ((msgsnd(msgid,(void *)&msg,strlen(msg.mdata)+1,0))<0) {
    perror("send msg to quenue");
    exit(2);
  } else {
    printf("send msg to quenue\n" );
  }
  return 0;
}
```

获取消息
```c

int main(int argc, char const *argv[]) {

  int msgid,length;
  struct mymsg{
    long mtype;
    char mdata[512];
  }msg;

  if ((msgid = msgget(3099,0666|IPC_CREAT))<0) {
    perror("create msg quenue");
    exit(1);
  } else {
    printf("create msg quenue\n" );
  }

  if ((length =msgrcv(msgid,(void *)&msg,512,0,0))<0) {
    perror("receive msg from quenue");
    exit(2);
  } else {
    printf("receive msg from quenue - msg type : %ld\n",msg.mtype );
    printf("receive msg from quenue - msg length : %d\n",length );
    printf("receive msg from quenue - msg mdata : %s\n",msg.mdata );
  }


  return 0;
}

```

**实际测试**
![send & receive message](https://img-blog.csdn.net/20180414161005994?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2VuZ2luZWVyX2phbWVz/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)
运行两次发送消息， type>0 返回type 为1 的第一个消息模式，然后运行接受消息两次 ，就可以获取两次消息，那么再次接受消息，会发现一直阻塞等待新的消息到来

以上是总结的linux 进程间通信的概念。

下一章准备学习线程，感觉也挺简单的:-)


---
[1]:https://blog.csdn.net/engineer_james/article/details/79897855
[2]:https://blog.csdn.net/engineer_james/article/details/79902399
[3]:https://download.csdn.net/download/engineer_james/10348756
