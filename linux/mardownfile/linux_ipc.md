#进程间通信
linux 系统中有很多进程，免不了进程间进行通信，即IPC通信，linux 中有6种方式
信号、无名管道(pipe)和有名管道(FIFO)、共享内存、信号量、消息队列、套接字(socket)

socket 已经在前面研究tcp/udp的时候学习过[socket tcp][1]、[socket udp][2]

## 信号
信号是软件中断产生，用于进程间异步传递信息
一般在shell 中操作，进程获取信号进行处理，一共有64中信号，在shell中输入 **kill -l** 可查阅

![signal](https://github.com/jamesyang1991CN/document/blob/master/picture/signal.png)


- 从来没有在这方面用过，主要的方法：

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

![pipe1](https://github.com/jamesyang1991CN/document/blob/master/picture/pipe1.png)

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

![piped](https://github.com/jamesyang1991CN/document/blob/master/picture/pipe_d.png)

**单工通信管道**

![piped](https://github.com/jamesyang1991CN/document/blob/master/picture/pipe_s.png)


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

![rdfifo](https://github.com/jamesyang1991CN/document/blob/master/picture/rdfifo.png)
![wrfifo](https://github.com/jamesyang1991CN/document/blob/master/picture/wrfifo.png)

## 共享内存 (敲黑板，划重点)
上面的通信需要经过内核，浪费系统资源，而共享内存就比较厉害，直接在用户态创建物理内存，不需要经过内核，支持大量数据传输

**共享内存的原理**：
在系统中取一块未使用的物理内存，然后两个进程独立的用户空间印射到这个物理内存，对应不同进程得到的是不同的内存地址，实现了两个进程对同一个物理内存读和写操作

因为不会涉及用户空间和内核空间的切换，所以效率高

![sharemomery](https://github.com/jamesyang1991CN/document/blob/master/picture/shm.png)


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

![shmwr](https://github.com/jamesyang1991CN/document/blob/master/picture/shmwr.png)

![shmrd](https://github.com/jamesyang1991CN/document/blob/master/picture/shmrd.png)


## 信号量

作用是为了保证 共享内存读取同步

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

而读进程 每隔3秒读取数据 显示

**直接测试**

![shmwr](https://github.com/jamesyang1991CN/document/blob/master/picture/shmwr2.png)

![shmrd](https://github.com/jamesyang1991CN/document/blob/master/picture/shmrd2.png)


上面的测试案例，告诉我们如果没有同步，由于发送端和读取端因为读取间隔不一样造成数据丢失，模拟了系统繁忙情况，所以同步很重要信号量作用很大，保证了数据同步




---
[1]:https://blog.csdn.net/engineer_james/article/details/79897855
[2]:https://blog.csdn.net/engineer_james/article/details/79902399
