#include <arpa/inet.h>
#include <stdio.h>
int main(int argc, char const *argv[]) {

  unsigned int i,num = 0xab127980;
  unsigned char * pc;

  printf("nums address is %p ,and it's value is 0x %x \n\n",&num,num );
  pc = (unsigned char *) &num;

  for (size_t i = 0; i < 4; i++) {
    /* code */
    printf("%p : 0x %x\n",pc,(unsigned int) *pc );
    pc++;
  }

  unsigned short port;
  port = 0x6789;

  printf("port number in host byteorder is 0x%x\n",port );
  printf("port number in network byteorder is 0x%x\n",htons(port) );

  return 0;
}


/*
%p 表示指针 指向的地址
nums address is 0x7ffc7ea363e4 ,and it's value is 0x ab127980

0x7ffc7ea363e4 : 0x 80
0x7ffc7ea363e5 : 0x 79
0x7ffc7ea363e6 : 0x 12
0x7ffc7ea363e7 : 0x ab
port number in host byteorder is 0x6789
port number in network byteorder is 0x8967ok



*/
