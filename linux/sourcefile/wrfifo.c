
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <time.h>
#include <signal.h>
# define PIPEBUF 1024;
void sig_pipe(int signo) {
  printf("i'm in sig_pipe\n");

  exit(-1);
}

int main(int argc, char const *argv[]) {

  int fd;
  int len;
  char buf[PIPEBUF];
  time_t tp;
  signal(SIGPIPE,sig_pipe);
  printf("i am %d\n",getpid() );
  if ((fd=open("fifo1",O_WRONLY))<0) {
    /* code */
    perror("open");
    exit(EXIT_FAILURE);
  }

  printf("between open & write\n");
  while (1) {
    /* code */
    time(&tp);
    len = sprintf(buf,"wrfifo %d sends %s ",getpid(),ctime(&tp));

    if ((write(fd,buf,len+1))<0) {
      /* code */
      perror("write");
      close(fd);
      exit(EXIT_FAILURE);
    }

    sleep(3);


  }
  close(fd);


  exit(EXIT_SUCCESS);



}
