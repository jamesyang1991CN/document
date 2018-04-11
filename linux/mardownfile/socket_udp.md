# udp 传输
在tcp传输后，如果需要传输大量的数据那就需要使用udp协议，使用的数据报格式 即 SOCK_DGRAM

的udp 编程模型

![socketudp](https://github.com/jamesyang1991CN/document/blob/master/picture/socket-udp.png)

**服务器端步骤**

第一步： 创建一个socket 套接口，返回一个文件描述符

第二步： 通过bind 绑定socket 本地的IP地址还有端口号

第三步： 直接用recvfrom从套接口接收客户端发送过来的数据 包含客户端的IP地址和端口号，如果没有发送就一直阻塞

第四步： 处理完数据，调用sendto将客户端的IP地址和端口参数传入

第五步：当处理完所有的数据，调用close关闭套接字

**客户端步骤**

第一步：创建socket

第二步：调用sendto将服务器端的IP地址和端口参数传入

第三步：等待数据返回，用recvfrom 接收数据

第四步：完成传输，调用close  关闭套接口


## 函数

函数原型：**sendto**
```c
ssize_t sendto(int sockfd,const void *buf,size_t len,int flag,const struct sockaddr *dest_addr,socklen_t addrlen);
/*
向指定接收端发送数据 成功返回发送的字节数 是被-1
sockfd 创建的socket 套接口文件描述符
buf 发送数据的内存首地址
len 发送数据的长度
flags 发送标志 一般为0
dest_addr 数据接收端的地址（IP地址和端口）结构体指针
addrlen 数据接收端地址结构体大小
*/

```

函数原型：**recvfrom**
```c
ssize_t recvfrom(int sockfd,void *buf,size_t len,int flags,struct sockaddr *src_addr,socklen_t *addrlen);
/*
接收数据，成功返回接收的数据字节数 反之-1
sockfd 接收使用的套接口文件描述符
buf 接受到数据存放内存中的位置
len 缓冲区的长度
flags 接收标志一般为0
src_addr 指向结构体 包含对端的地址和端口
addrlen 指向对象地址的实际大小
*/

```

## 实际编写测试
代码主要功能 客户端直接发送消息给服务器端，然后返回，但是我在测试的时候，才发现会丢包还挺严重

**server_udp.c**

```c
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

```

**client_udp.c**
```c
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

```

[全部源码点击点击这里][1]

---
[1]:https://download.csdn.net/download/engineer_james/10342302
