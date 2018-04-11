
#include <stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
/*
struct hostent{
  char * h_name; //主机的正式名称
  char ** h_aliases; // 主机的别名
  char h_addrtype; //主机的地址类型 AF_INET
  char h_length; //主机的地址长度 ipv4
  char **h_addr_list; 主机的地址列表



}

struct in_addr {

    in_addr_t s_addr;

};

*/
int main(int argc, char const *argv[]) {

  struct hostent  *val_hostent;
  struct hostent  *val_hostent_addr;
  struct in_addr addr;
  val_hostent = gethostbyname("james-ubuntu");

  printf("hostname = %s  hostaliases = %s  addrtype=%d  length=%d h_addr = %s\n",val_hostent->h_name,*val_hostent->h_aliases,val_hostent->h_addrtype,val_hostent->h_length,val_hostent->h_addr_list[0] );

   addr.s_addr= inet_addr("192.168.0.74");//点分十进制的IP转换成一个长整数型数 in_addr_t inet_addr(const char *cp);
    printf("addr = %ud  \n",addr.s_addr);

//struct hostent *gethostbyaddr(const char * addr, int len, int type);
  val_hostent_addr = gethostbyaddr((const char*)&addr.s_addr, sizeof(addr.s_addr), AF_INET); //??????
      // printf("host name:%s   addr %s\n", val_hostent_addr->h_name,&addr);
  // printf("hostname = %s   \n",val_hostent_addr->h_name);


  if (val_hostent_addr == NULL) {
    /* code */
    perror("val_hostent_addr");
  }
  // printf("hostname = %s   \n",val_hostent_addr->h_name);
  char* ss = inet_ntoa(addr);//char * inet_ntoa(struct in_addr in);
  printf("addr = %s  \n",ss);//返回点分十进制的字符串在静态内存中的指针

  return 0;
}
