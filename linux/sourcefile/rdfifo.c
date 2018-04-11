
#include<sys/types.h>
#include<sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
# define PIPEBUF 1024

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
    /* code */
    printf("open error %d\n",fd);
    exit(1);
  }

  while ((len = read(fd,buf,PIPEBUF-1))>0) {
    /*
    参数fd所指的文件传送nbyte个字节到buf指针所指的内存中。
    若参数nbyte为0，则read()不会有作用并返回0。
    返回值为实际读取到的字节数，如果返回0，
    表示已到达文件尾或无可读取的数据。错误返回-1,
    并将根据不同的错误原因适当的设置错误码。
    */
    /* code */
    printf("rdfifo read %s\n",buf );
  }
  printf("%d\n", len);

  close(fd);
  if (len == 0) {
    /* code */
    printf("read end\n");
  }

  exit(EXIT_SUCCESS);


  return 0;
}
