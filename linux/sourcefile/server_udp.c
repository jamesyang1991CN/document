
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>

# define SERVER_PORT 8888
# define MAX_MSG_SIZE 1024

char host_name[20];

void udp_res(int sockfd){

  struct sockaddr_in addr;
  int n,n1;
  unsigned int addrlen;
  char msg[MAX_MSG_SIZE];
  char sendbuf[MAX_MSG_SIZE];
  while (1) {
    /*recv 接收数据*/
    if ((n = recvfrom(sockfd,msg,MAX_MSG_SIZE,0,(struct sockaddr *)&addr,&addrlen))<0) {
      perror("receive from client data");
      exit(3);
    }else{
      printf("receive from client data...\n" );
    }

    inet_ntop(AF_INET,&addr.sin_addr,host_name,sizeof(host_name));

    printf("receive from client (%s):%s\n",host_name, msg);

    /*处理接收到的msg 信息*/
    int sum = 0;
    for (int i = 0; i < sizeof(msg); i++) {

      if(msg[i] == 'y'){
        sum++;
      }
    }

    char* sumstr = (char *)malloc(sizeof(int) + 1);  //分配动态内存
    memset(sumstr, 0, sizeof(int) + 1);              //内存块初始化
    sprintf(sumstr, "%d", sum);                  //整数转化为字符串

    printf("sum = %d sumstr = %s\n",sum,sumstr);
    strcpy(sendbuf,"udp server receive msg , has");
    strcat(sendbuf,sumstr);
    free(sumstr);
    strcat(sendbuf," y");
    printf("sendbuf: %s\n",sendbuf);
    /*发送数据*/
    if ((n1 = sendto(sockfd,sendbuf,sizeof(sendbuf),0,(struct sockaddr *)&addr,addrlen))<0) {
      perror("send data to client");
    }else{
      printf("send data (%d)%s to client.....\n",n1,sendbuf );
    }
  }


}




int main(int argc, char const *argv[]) {
  int sockfd;
  struct sockaddr_in addr;
  /*创建套接口文件描述符*/
  if ((sockfd = socket(AF_INET,SOCK_DGRAM,0))<0) {
    perror("create socket...");
    exit(1);
  }else{
    printf("create socket ...\n");
  }

  bzero(&addr,sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(SERVER_PORT);

  if (bind(sockfd,(struct sockaddr *)&addr,sizeof(struct sockaddr_in))<0) {
    perror("bind to sockaddr");
    exit(2);
  } else {
    printf("bind to socket addr\n");
  }

  udp_res(sockfd);

  close(sockfd);

  return 0;
}
