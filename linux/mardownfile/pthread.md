# 线程和进程
这个概念不管在什么操作系统中都是一样的，也是面试官比较喜欢问的，代表你对程序优化的功底，搞安卓的时候，经常被用来优化处理速度 还有数据的处理，结合handler 一起处理，解决经常出现界面死掉问题。

既然总结了，这一次充分总结下：
## 进程
进程组成：
  1. 进程控制块PCB
  2. 程序段
  3. 数据段

进程控制块PCB 是内核中存放的一块PCB区域，记录型数据结构 ，PCB 记录了操作系统所需要的参数，用来描述进程目前情况和进程运行的全部信息，包括
  1. 进程描述信息：
    1. 进程标识符，识别进程
    2. 用户标识，用于资源共享和保护
    3. 家族关系，进程关于有父进程和子进程的信息
  2. 处理机状态信息：包含通用寄存器、指令寄存器、程序状态字（PSW）、用户栈指针等
    1. 进程状态 ，用作进程调度的依据
    2. 进程的优先级，处理机运行进程的依据
    3. 进程调度需要的数据： 比如已经等待CPU的总和、进程已经执行的时间总和
    4. 事件： 进程被阻塞的原因
  3. 进程控制信息
    1. 程序和数据的地址
    2. 进程和同步通信机制

进程的状态： 就绪、执行、阻塞

![process](https://github.com/jamesyang1991CN/document/blob/master/picture/process.png)

上面是进程的各种状态切换

当第一初始化进程的时候，进程进入就绪状态，等待OS 分配处理机处理进程，当进程获取处理机处理，进程执行阶段，执行的时候，一般会用来处理各种事物，比如 请求 IO 处理等，但是一旦有大量IO 处理，容易进入阻塞状态，此时就是我们经常看见电脑卡死的状态，但是处理完后，进程会进入就绪状态
其实还有一种状态，finish 结束状态，但是只要进程死了就直接退出了


## 线程
从上面很容易看出来，创建一个进程，因为直接和linux 内核进行处理数据，需要提供大量的参数，来保障进程的安全，高效，稳定，消耗很多系统资源，但是线程就不一样，可以原理是一样的，作为轻量级，消耗系统资源很少，而且作为开发者来说，线程用起来更爽，更方便，线程只能跑在进程中，所以，和系统内核交互的数据都交给进程了，线程简单方便易用，据说进程消耗资源是线程的30倍.....
线程和处理流程和 进程差不错 ，也是 就绪、执行、阻塞、结束四个状态

**多线程编程的API**

创建线程

```c
int pthread_create(pthread_t *thread,pthread_attr_t *attr,void * (*func)(void * ),void *arg);
/*
看了上面的方法，mmp、wtf 这些词汇从我脑海中冒出来，函数参数要不要写的那么复杂
只能看调用示例了：
pthread(&thrd1,NULL,(void*)task1,(void*)&g1);
phtread thrd1 表示创建线程的标识
pthread_attr_t *attr 表示线程属性NULL 默认
void task1 线程需要执行的代码
int g1 表示task1的参数
*/
```

结束线程

```c
pthread_exit(void *retval);
//retval 用来存放自杀线程退出状态
```

等待线程结束

```c
// 多个线程启动后，由系统负责调度，但我们并不知道哪个会先开始和结束，只能等待
int pthread_join(pthread_t th,void **thread_return);
/*
th 等待线程的标识
thread_return 返回的状态
*/

```
**多线程实例：**

1. 只有同步锁的情况，线程之间的同步互斥

用同步互斥原理，实现对变量shareid的操作

```c
int shareid=0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int main(int argc, char const *argv[]) {
  pthread_t thrd1,thrd2;
  int ret;
  ret = pthread_create(&thrd1,NULL,(void *)task1,NULL);//创建线程
  ret = pthread_create(&thrd2,NULL,(void *)task2,NULL);
  pthread_join(thrd1,NULL);//等待线程结束
  pthread_join(thrd2,NULL);
  printf("shareid = %d\n",shareid );
  return 0;
}
  void task1(void) {
    long i,tmp;
    for ( i = 0; i < 10000; i++) {
      if (pthread_mutex_lock(&mutex)!=0) {//加同步锁
        perror("pthread_mutex_lock");
          exit(EXIT_FAILURE);
      }
      tmp = shareid;
      tmp = tmp+1;
      shareid = tmp;
      if (pthread_mutex_unlock(&mutex)!=0) {//解锁
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


```

因为 pthread 不是gcc 编译标准库，需要用到动态库加载，编译编译命令最后 加上-lpthread
> gcc pthread_create.c -o pthread_create -lpthread

**debug 结果:**

![ph_create](https://github.com/jamesyang1991CN/document/blob/master/picture/threadcreate.png)

2. 线程+同步锁+信号量 会有更有意思

很明显同步锁，有点死板，每次只能操作一个变量，不能实现多个公共资源的操作，但是加上信号量就有意思了

**semaphore API 介绍**

创建信号量
```c
#include <semaphore.h>
int sem_init(sem_t *sem,int pshared,unsigned int value);
/*
功能： 初始化 信号量
返回值： 成功返回0 错误返回-1
sem： 指向信号量结构的指针
pshared 不为0 信号量在进程间共享，不然只能在本进程中的所有线程中共享
value 给出信号量的初始值
*/
```
信号量 P操作 -1
```c
int sem_wait(sem_t *sem);
```
信号量 V操作 +1
```c
int sem_post(sem_t *sem);
```
信号量删除
```c
int sem_destroy(sem_t *sem);
```

看了上面的API，和进程中用的API 不一样? [linux ipc 进程间通信总结][1]
我只能这么理解了，进程和线程适用不同的信号量API

案例如下：

```c
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
```
有一个线程th3对 stack 循环赋值，并且每赋值一次 将信号量sem V操作，thd1和thd2这两个线程会竞争获取stack 操作 thd1 是求和，而thd2是求商，哪一个线程获取资源了，求和或者求商，然后对sem进程P操作，所以每次运行程序会打印不同的结果

这里还有一个知识点， 逗号运算符
```c
  while (pthread_mutex_lock(&mutex),((i =size++)<MAXSIZE)) {
    pthread_mutex_unlock(&mutex);
```
 while 有两个表达式，最后只判断((i =size++)<MAXSIZE) 是否符合，前面只是对mutex 进行加锁操作，这样只是为了保证size 能正常执行++操作和判断

 逗号表达式： 逗号前面和后面的表达式都执行，但是只有最后一个返回值有效




 ## 其他unbelieve知识点
 **pthread_create 上面第二个参数一直设置NULL 系统默认属性，但是如果我们设置，该如何设置呢？**

> 如果设置线程属性，必须在**pthread_create **之前使用**pthread_attr_init**.<br/>
> pthread_attr_t 结构体包含 是否绑定，是否分离，堆栈地址，堆栈大小，优先级信息<br/>
> 默认属性是 非绑定 非分离 默认1M大小堆栈 优先级和进程一样<br/>

** 轻进程概念，理解为内核进程，位于用户层和系统层之间，系统对线程资源的分配通过轻进程实现，如果设置绑定在轻进程，那个线程响应度高。**

**绑定状态**
> 设置绑定状态的函数 **pthread_attr_setscope** <br/>
> 绑定：PTHREAD_SCOPE_SYSTEM 非绑定 PTHREAD_SCOPE_PROCESS<br/>

**分离状态**
> 线程的分离状态，用来决定如何结束自己
> 非分离状态使用pthread_join 处理，才能释放资源，分离状态的线程，运行结束，自动释放<br/>
> 非分离：PTHREAD_CREATE_JOINABLE 分离：PTHREAD_CREATE_DETACHED 设置参数的函数 **pthread_attr_setdetachstate**<br/>

> 如果设置线程分离状态，线程运行太快,在pthread_attr_init 之后，但是在pthread_create之前结束，那么此时创建线程，会得到错误线程号，需要避免，最简单的方法 执行pthread_cond_timewait 使得线程慢点跑

**线程优先级**
> 线程优先级 存放在结构体sched_param 用**pthread_attr_getsparam** 函数 和**pthread_attr_setschedpara** 存放， 先取得优先级，修改后，存放回去


 **线程创建后改变属性：**

 > 杀死其他线程的方法 **pthread_cancel**(thread)

 但是线程可以设置属性 拒绝被杀死
> pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL)
> pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL)

---
[1]:https://blog.csdn.net/engineer_james/article/details/79938646
