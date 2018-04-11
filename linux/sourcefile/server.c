#include <sys/types.h>
#include <sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

# define BUFSZ 2048
char sendbuf[BUFSZ];
char recvbuf[BUFSZ];

char host_name[20];
int port = 8000;

int main(int argc, char const *argv[]) {
  struct sockaddr_in server_in, client_in;
  int sockfd,client_sockfd;
  /*创建socket ipv4 tcp 链接*/
  if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
    perror("create socket...");
    exit(1);
  }else{
    printf("create socket success ...\n");
  }


  bzero(&server_in,sizeof(server_in));
  /*
  #include <string.h>　　功能：置字节字符串s的前n个字节为零且包括‘\0’ 无返回值
  void *memset(void *s, int ch, size_t n);
  将s中前n个字节替换为ch并返回s
  它是对较大的结构体或数组进行清零操作的一种最快方法
  */
  server_in.sin_family = AF_INET;
  server_in.sin_addr.s_addr = INADDR_ANY;
  server_in.sin_port = htons(port);//将主机字节顺序变成网络字节顺序

  /*开始绑定服务器的地址和端口 还有协议族 */
  if ((bind(sockfd,(struct sockaddr *)&server_in,sizeof(server_in)))==-1) {
    perror("call bind");
    exit(2);
  }else{
    printf("bind local socket IP address and port ...\n");
  }

  /*开始监听套接口 设置监听队列100 */
  if ((listen(sockfd,100) ) == -1) {
    perror("call listen");
    exit(3);
  }else{
    printf("listen socket ...\n");
  }

  printf("accepting connections ...\n");

  /*while 保持接收客户端的请求，如果没有就保持阻塞状态*/
  while (1) {
      int client_addrlen = sizeof(client_in);

      /*开始accept 客户端链接获取客户端IP地址端口号 */
      if ((client_sockfd = accept(sockfd,(struct sockaddr *)&client_in,&client_addrlen))==-1) {
        perror("call accept get client ip and port");
        exit(4);
      } else {
        printf("accept client request...\n");
      }
      /*使用连接返回套接字描述符 进行发送消息*/
      if ((recv(client_sockfd,recvbuf,sizeof(recvbuf),0))==-1) {
        perror("recv buf message");
        exit(5);
      } else {
        printf("receive buffer...\n");
      }

      inet_ntop(AF_INET,&client_in.sin_addr,host_name,sizeof(host_name));

      printf("received from client (%s):%s\n",host_name,recvbuf);

      /*处理接收到的buf 信息*/
      int sum = 0;
      for (int i = 0; i < sizeof(recvbuf); i++) {

        if(recvbuf[i] == 'y'){
          sum++;
        }
      }

      char* sumstr = (char *)malloc(sizeof(int) + 1);  //分配动态内存
      memset(sumstr, 0, sizeof(int) + 1);              //内存块初始化
      sprintf(sumstr, "%d", sum);                  //整数转化为字符串

      printf("sum = %d sumstr = %s\n",sum,sumstr);

      strcpy(sendbuf,"i get your message has ");
      strcat(sendbuf,sumstr);
      free(sumstr);
      strcat(sendbuf," y");
      printf("sendbuf: %s\n",sendbuf);
      /*将消息发送给客户端*/
      if ((send(client_sockfd,sendbuf,sizeof(sendbuf),0))==-1) {
        perror("send buf message");
        exit(6);
      } else {
        printf("send buffer...\n");
      }
      printf("close connections...\n");

  }

  close(client_sockfd);
  return 0;
}
