
#include <sys/types.h>
#include <sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

# define BUFSZ 4096
char recvbuf[BUFSZ];
char sendbuf[BUFSZ];

/*服务器端的地址*/
char *hostname = "127.0.0.1";
int port = 8000;

int main(int argc, char *argv[]) {

  int serversockfd,client_sockfd;
  struct sockaddr_in client_in,server_in;
  char * str = "asdsaddyyddddddyydasafdyygfdavas";
  if (argc<2) {
    printf("we send default test\n");
    printf("Usage: client   any_string  ip_address \n" );
  } else {
    str = argv[2];
    hostname = argv[1];

  }

  /*创建socket */
  if ((client_sockfd = socket(AF_INET,SOCK_STREAM,0))==-1) {
    perror("create client socket");
    exit(1);
  }else{
    printf("create client socket...\n");

  }

  bzero(&server_in,sizeof(server_in));
  server_in.sin_family = AF_INET;
  server_in.sin_port = htons(port);
  inet_pton(AF_INET,hostname,&server_in.sin_addr);//将hostname 转化为　网络字节
  /*开始连接服务器端*/
  if ((connect(client_sockfd,(struct sockaddr *)&server_in,sizeof(server_in)))==-1) {
    perror("start connect to server...");
    exit(2);
  } else {
    printf("start connect to server...\n");
  }
  strcpy(sendbuf,str);
  /*write 数据*/
  printf("start to send message %s to server\n",sendbuf);

  if (write(client_sockfd,sendbuf,sizeof(sendbuf))==-1) {
    perror("start send message to server...");
    exit(3);
  } else {
    printf("start send message to server..\n");
  }

  printf("..send message .....wait for response...\n");

  /*read message from server*/

  if (read(client_sockfd,recvbuf,sizeof(recvbuf))==-1) {
    perror(" response from server\n ");
    exit(4);
  } else {
    printf("\n response from server:\n %s\n ",recvbuf);
  }
  close(client_sockfd);
  printf("close connections...\n");
  return 0;
}
