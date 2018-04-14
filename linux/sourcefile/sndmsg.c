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

  msg.mtype = 1 ;
  strcpy(msg.mdata,"hello james\n");

  if ((msgsnd(msgid,(void *)&msg,strlen(msg.mdata)+1,0))<0) {
    perror("send msg to quenue");
    exit(2);
  } else {
    printf("send msg to quenue\n" );
  }
  return 0;
}
