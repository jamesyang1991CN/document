

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

# define BUFSZ 4096


int main(int argc, char const *argv[]) {

  int shmid;
  char * shmbuf;


  if ((shmid = shmget(888,BUFSZ,0666|IPC_CREAT))<0) {
    /* code */
    perror("shmget read");
    exit(EXIT_FAILURE);
  }
printf("shmid %d\n", shmid);
  if ((shmbuf = shmat(shmid,0,0))< (char *)0) {
    /* code */
    perror("shmat read");
    exit(EXIT_FAILURE);
  }


  printf("shmbuf %s\n", shmbuf);

// write(STDOUT_FILENO,shmbuf,1);


while(1){
  printf("in while \n");

  write(STDOUT_FILENO,shmbuf,1);
  write(STDOUT_FILENO,"\n",1);
  sleep(3);


}




  exit(EXIT_SUCCESS);
}
