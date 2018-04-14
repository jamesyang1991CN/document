#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[]) {

  int msgid,length;
  struct mymsg{
    long mtype;
    char mdata[512];
  }msg;

  if ((msgid = msgget(3099,0666|IPC_CREAT))<0) {
    perror("create msg quenue");
    exit(1);
  } else {
    printf("create msg quenue\n" );
  }

  if ((length =msgrcv(msgid,(void *)&msg,512,0,0))<0) {
    perror("receive msg from quenue");
    exit(2);
  } else {
    printf("receive msg from quenue - msg type : %ld\n",msg.mtype );
    printf("receive msg from quenue - msg length : %d\n",length );
    printf("receive msg from quenue - msg mdata : %s\n",msg.mdata );
  }


  return 0;
}
