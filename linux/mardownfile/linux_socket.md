# linux

最近一直再看[gstreamer][1],使用了很多插件和库，虽然我总结了一部分rtsp流程，对于这么大的框架，明显感觉分析不了，比如linux API、Gobject（Glib库）推荐一本入门书籍《linux下c编程》，有案例可以debug


# socket

socket 是网络编程一个最基本的函数，本地应用，远端应用都可以用来通信，基于 tcp 和 udp 传输
## socket 通信模型
![socket](https://github.com/jamesyang1991CN/document/blob/master/picture/socket-server.png)

**服务器端**

第一步：创建socket （与网络连接的套接口），返回一个文件描述符，socket 物理上表现为网络连接，编程上表现为文件描述符。

第二步: bind 绑定 创建socket和本机的IP地址和端口号

第三步: 调用listen 指定socket为被动套接口，监听网络到达的请求

第四步: 调用accept使得socket可以接收网络请求，如果没有连接请求，socket 进入 阻塞状态，，反之返回一个连接套接口socket （此时TCP三次握手已经完成）

第五步: read 读取数据

第六步: 处理请求，根据请求使用write发送数据

第七步: 最后调用close 关闭连接套接口

**客户端**

第一步: 创建一个socket

第二步: 调用connect 传入参数服务器的 IP PORT 发起连接,当三次握手完成后，connect 会返回

第三步: write 向socket 写入数据，发送请求

第四步: 当服务器返回数据，使用read 读取数据内容

第五步: 最后通过close 关闭socket




## 函数说明
想使用socket 必须包含 头文件
```c
#include <sys/tytpes.h>
#include <sys/socket.h>
```
函数原型：**socket**

```c
int socket(int family, int type, int protocol);

/*
return 返回文件描述符 fd
family 协议族  地址族 AF_INET 表示ipv4
type 的服务类型 TCP对应SOCK_STREAM 表示流  UDP对应 SOCK_DGRAM 表示数据报
protocol用0表示默认family 和 type

*/

```
函数原型：**bind**
```c
int bind(int sockfd,const struct sockaddr * localaddr,socklen_t addrlen);
/*
return 成功返回0 反之-1
sockfd socket 创建的套接字描述符
localaddr 指定本地的IP地址和端口
addrlen 地址结构的字节数大小
*/
```

函数原型：**listen**

```c
int listen(int sockfd,int queuelen);
/*
return 成功返回0 反之-1
sockfd socket创建的套接口
queuelen 请求对的大小
*/
```

函数原型：**accept**
```c
int accept(int sockfd,struct sockaddr *addr,socklen_t *addrlen);
/*
return 返回套接字描述符
sockfd socket 创建的套接字描述符
addr 地址结构的指针 accept 后，包含远程的IP地址和协议端口号
addrlen addr结构大小的存放地址，调用结束后获取远程的socket实际地址大小
*/
```

函数原型: **connect**
```c
int connect(int sockfd,const struct sockaddr *addr,socklen_t addrlen);
/*
return 成功返回0 反之-1
sockfd 套接字的描述符
addr 远程机器端点的地址
addrlen 是struct sockaddr 结构体的大小
*/
```

函数原型：send
```c
int send( SOCKET sockfd,char *buf,int len,int flags );
/*
sockfd: 接收端套接字描述符
buf：   用来存放send函数的数据的缓冲区
len: 指明buff的长度
flags:  一般置为0
*/
```
函数原型：recv
```c
int recv( SOCKET sockfd, char *buf, int  len, int flags);
/*
sockfd: 接收端套接字描述符
buf：   用来存放recv函数接收到的数据的缓冲区
len: 指明buff的长度
flags:  一般置为0
*/
```

```c
struct sockaddr{
  unsigned short sa_family;//address family AF_XXX 占用2个字节
  char sa_data[14]; //14bytes of protocol address 占用 14个字节
}
//一般不用上面的数据结构，而是使用等价的数据结构
//#include<netinet/in.h>
struct sockaddr_in{
  short int sin_family; //address family  占2个字节
  unsigned short int sin_port; //port number 占2个字节
  struct in_addr sin_addr; //internet address 占用4个字节
  unsigned char sin_zero[8]; //same size as struct sockaddr 占8个字节
}

struct in_addr {

    in_addr_t s_addr; //占用4个字节

};
```
## demo

**环境**：ubuntu 16.04 gcc 编译

服务器端实现接收客户端数据，并且对消息处理获取有多少个‘y’，把结果返回给客户端

客户端实现，发送一段字符串消息，接收客户端返回的消息


**server.c**

这里显示无注释版本，[注释版本][2]
```c
int main(int argc, char const *argv[]) {
  struct sockaddr_in server_in, client_in;
  int sockfd,client_sockfd;

  if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1) {
    perror("create socket...");
    exit(1);
  }else{
    printf("create socket success ...\n");
  }


  bzero(&server_in,sizeof(server_in));
  server_in.sin_family = AF_INET;
  server_in.sin_addr.s_addr = INADDR_ANY;
  server_in.sin_port = htons(port);

  if ((bind(sockfd,(struct sockaddr *)&server_in,sizeof(server_in)))==-1) {
    perror("call bind");
    exit(2);
  }else{
    printf("bind local socket IP address and port ...\n");
  }

  if ((listen(sockfd,100) ) == -1) {
    perror("call listen");
    exit(3);
  }else{
    printf("listen socket ...\n");
  }

  printf("accepting connections ...\n");

  while (1) {
      int client_addrlen = sizeof(client_in);


      if ((client_sockfd = accept(sockfd,(struct sockaddr *)&client_in,&client_addrlen))==-1) {
        perror("call accept get client ip and port");
        exit(4);
      } else {
        printf("accept client request...\n");
      }

      if ((recv(client_sockfd,recvbuf,sizeof(recvbuf),0))==-1) {
        perror("recv buf message");
        exit(5);
      } else {
        printf("receive buffer...\n");
      }

      inet_ntop(AF_INET,&client_in.sin_addr,host_name,sizeof(host_name));

      printf("received from client (%s):%s\n",host_name,recvbuf);

      int sum = 0;
      for (int i = 0; i < sizeof(recvbuf); i++) {

        if(recvbuf[i] == 'y'){
          sum++;
        }
      }

      char* sumstr = (char *)malloc(sizeof(int) + 1);
      memset(sumstr, 0, sizeof(int) + 1);
      sprintf(sumstr, "%d", sum);
      printf("sum = %d sumstr = %s\n",sum,sumstr);
      strcpy(sendbuf,"i get your message has ");
      strcat(sendbuf,sumstr);
      free(sumstr);
      strcat(sendbuf," y");
      printf("sendbuf: %s\n",sendbuf);
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

```
log:
> create socket success ...<br/>
> bind local socket IP address and port ...<br/>
> listen socket ...<br/>
> accepting connections ...<br/>
> accept client request...<br/>
> receive buffer...<br/>
> received from client (127.0.0.1):asdsaddyyddddddyydasafdyygfdavas<br/>
> sum = 6 sumstr = 6<br/>
> sendbuf: i get your message has 6 y<br/>
> send buffer...<br/>
> close connections...<br/>


**client.c**

这里显示无注释版本，[注释版本][2]

```c
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
  if ((connect(client_sockfd,(struct sockaddr *)&server_in,sizeof(server_in)))==-1) {
    perror("start connect to server...");
    exit(2);
  } else {
    printf("start connect to server...\n");
  }
  strcpy(sendbuf,str);
  printf("start to send message %s to server\n",sendbuf);

  if (write(client_sockfd,sendbuf,sizeof(sendbuf))==-1) {
    perror("start send message to server...");
    exit(3);
  } else {
    printf("start send message to server..\n");
  }
  printf("..send message .....wait for response...\n");
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

```
log:
> we send default test <br/>
> Usage: client   any_string  ip_address<br/>
> create client socket...<br/>
> start connect to server...<br/>
> start to send message asdsaddyyddddddyydasafdyygfdavas to server<br/>
> start send message to server..<br/>
>..send message .....wait for response...<br/>
>
 > response from server:<br/>
 > i get your message has 6 y<br/>
 > close connections...<br/>

上面就是一个简单的socket 连接，自己动手写一写能学习到很多东西

## 实际测试显示
![server](https://github.com/jamesyang1991CN/document/blob/master/picture/socket_server.png)

![client](https://github.com/jamesyang1991CN/document/blob/master/picture/socket_client.png)
---
[1]:https://gstreamer.freedesktop.org/
[2]:https://download.csdn.net/download/engineer_james/10341595
