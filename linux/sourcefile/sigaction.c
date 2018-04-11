
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

static void sig_int(int signo){
	int i;
	if (signo == SIGINT) {
		/* code */
		printf("received SIGINT\n" );
		for (size_t i = 0; i < 100000000; i++);
		printf("processed SIGINT\n");

	}else if (signo == SIGQUIT) {
		/* code */
		printf("caught SIGQUIT\n");
	} else {
		/* code */
		printf("received signal %d \n",signo );
	}

	return;
}

int main(int argc, char const *argv[])
{
	/* code */

	struct sigaction act;

	sigset_t newmask,oldmask,pendmask;
/*
	if (signal(SIGQUIT,sig_int) == SIG_ERR)//信号处理函数 SIGINT
	{

		printf("can't catch SIGQUIT\n");
		exit(1);
	}

	if (signal(SIGINT,sig_int) == SIG_ERR)
	{

		printf("can't catch SIGINT\n");
		exit(2);
	}

	sigemptyset(&act.sa_mask); //sa_mask 每一位置0
	sigaddset(&act.sa_mask,SIGQUIT);//sa_mask 中 SIGQUIT置1
	act.sa_handler = sig_int;
	act.sa_flags = 0;
*/
/*
	if (sigaction(SIGINT,&act,NULL) == -1) { //改变进程接收到指定信号的行动 SIGINT 中断信号 ctrl+c
																					//新的行动  act   NULL 用来保存旧的行动信号

		printf("can't catch SIGINT\n");
		exit(3);
	}
*/
	sigemptyset(&newmask);
	sigaddset(&newmask,SIGQUIT);//SIGQUIT开启


	if (sigprocmask(SIG_BLOCK,&newmask,&oldmask)<0) {//并集  获取和改变进程信号掩码

		printf("SIG_BLOCK  error\n " );
		exit(4);
	}
	sleep(15);


	if (sigpending(&pendmask)<0) {//获取正在阻塞的信号 放入pendmask

		printf("sigpending error\n");
		exit(5);
	}
	if (sigismember(&pendmask,SIGQUIT)<0) {

		printf("\nSIGQUIT pending \n");
	}

	if (sigprocmask(SIG_SETMASK,&oldmask,NULL)<0) {//并集  获取和改变进程信号掩码

		printf("SIG_SETMASK  error\n " );
		exit(6);
	}

	printf("SIGQUIT unblocked\n");
	sleep(100);

	exit(0);
}
