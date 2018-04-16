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


```
debug 结果
