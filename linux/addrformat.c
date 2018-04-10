
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>

int main(int argc, char const *argv[]) {

  int addrnum;
  char addrstring[16] = "10.238.100.43";
  printf("address in dotted-quad format is %s\n",addrstring );

  inet_pton(AF_INET,addrstring,&addrnum);
  printf("address in network byteorder integer is 0x %x\n",addrnum );

  char addrstring2[16] = "2.2.2.2";
  if (inet_ntop(AF_INET,&addrnum,addrstring2,16)==NULL) {
    /* code */
    perror("inet_ntop");
  }

  printf("address in dotted-quad format is %s\n", addrstring2);


  


  return 0;
}
