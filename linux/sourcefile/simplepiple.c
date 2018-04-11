#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
# define BUFSIZE 1024

int main(int argc, char const *argv[]) {
  /* code */
  int pid,fd[2],n;
  char buf[BUFSIZE];

  if (pipe(fd) < 0 ) {
    /* code */
    printf("pipe error \n");
    exit(1);
  }

  if ((pid = fork()) < 0 ) {
    /* code */
    printf("fork failure\n");
    exit(1);
  }else if (pid == 0) { //child
    /* code */
    close(fd[1]);//关闭管道 写端
    while ((n = read(fd[0],buf,BUFSIZE)) >0 ) {//读取 子进程读端数据
      /* code */
      write(STDOUT_FILENO,"receive from parent: ",21);
      write(STDOUT_FILENO,buf,n);
      printf("%s\n", buf);
      char *str = buf;
      if (strcmp("q",str) == 0) { //功能暂时无效
        /* code */
        printf("exit \n");
        exit(0);

      }
    }

    if (n < 0 ) {
      /* code */
      printf("read error\n");
      exit(1);
    }
    close(fd[0]);
    printf("exit child\n");
    exit(0);
  }
  close(fd[0]);
  while ((n = read(STDIN_FILENO,buf,BUFSIZE)) >0 )
    /* code */
    write(fd[1],buf,n);

  if (n < 0 ) {
    /* code */
    printf("write error\n");
    exit(1);
  }
  close(fd[1]);

  printf("exit parent\n");

  return 0;
}
