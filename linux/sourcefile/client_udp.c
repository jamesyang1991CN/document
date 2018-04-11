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
# define MAX_MSG_SIZE 1024

/*服务器端的地址*/
char *hostname = "127.0.0.1";
int port = 8888;
char * str = "asdsaddyyddddddyydasafdyygfdavas";
void udp_request(int sockfd,const struct sockaddr_in * addr,int len){

  char buffer[MAX_MSG_SIZE],recvbuf[MAX_MSG_SIZE];
  int n,n1;
  while (1) {
    fgets(buffer,MAX_MSG_SIZE,stdin);
    if (NULL == buffer) {
      /* code */
      strcpy(buffer,str);
    }
    /*发送数据*/
    if ((n =sendto(sockfd,buffer,strlen(buffer),0,(struct sockaddr *)addr,len))<0) {
      perror("sendto server");
      exit(3);
    } else {
      printf("sendto server %d bytes ....\n",n );
    }

    printf("sendto server data %s .....\n",buffer );

    printf("waiting to response from server....\n" );

    /*接收数据*/
    if ((n1 =recvfrom(sockfd,recvbuf,strlen(recvbuf),0,NULL,NULL))<0) {
      perror("receive data from server");
      exit(4);
    } else {
      printf("receive data from server %d bytes ,data: %s ....\n",n1,recvbuf );
    }
  }


}


int main(int argc, char *argv[]) {

  int sockfd;
  struct sockaddr_in addr_in;

  if (argc<2) {
    printf("we send default test\n");
    printf("Usage: client   any_string  ip_address \n" );

  } else {
    str = argv[2];
    hostname = argv[1];

  }

  /*创建socket*/
  if ((sockfd = socket(AF_INET,SOCK_DGRAM,0))<0) {
    perror("create socket");
    exit(2);
  } else {
    printf("create socket....\n" );
  }

  bzero(&addr_in,sizeof(struct sockaddr_in));
  addr_in.sin_family = AF_INET;
  addr_in.sin_port = htons(port);
  inet_pton(AF_INET,hostname,&addr_in.sin_addr);

  udp_request(sockfd,&addr_in,sizeof(struct sockaddr_in));

  close(sockfd);

  return 0;
}
