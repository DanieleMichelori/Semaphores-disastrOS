#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS.h"

#define BUFFER_LENGTH 50
#define ITERATION 10

#define PRODUCER_ID 1
#define CONSUMER_ID 2
#define WRITE_ID 3
#define READ_ID 4

int buffer[BUFFER_LENGTH];
int write_index = 0;
int read_index = 0;
int cnt = 0;

void producer_function(int filled_sem, int empty_sem, int read_sem, int write_sem) { //enqueue
  disastrOS_semWait(empty_sem);
  disastrOS_semWait(write_sem);

  printf("[WRITE] Scrivo nel buffer alla cella %d il valore %d\n", cnt, write_index);
	buffer[write_index] = cnt;
	write_index = (write_index + 1) % BUFFER_LENGTH;
	cnt++;

  disastrOS_sleep(1);

  disastrOS_semPost(write_sem);
  disastrOS_semPost(filled_sem);
}

void consumer_function(int filled_sem, int empty_sem, int read_sem, int write_sem) {  //dequeue
  disastrOS_semWait(filled_sem);
  disastrOS_semWait(read_sem);

  int x = buffer[read_index];
  printf("[READ] Leggo nel buffer alla cella %d il valore %d\n", x, read_index);
	read_index = (read_index + 1) % BUFFER_LENGTH;

  disastrOS_sleep(1);

  disastrOS_semPost(read_sem);
  disastrOS_semPost(empty_sem);
}

// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

void childFunction(void* args){
  printf("Hello, I am the child function %d\n",disastrOS_getpid());
  printf("I will iterate a bit, before terminating\n");
  int type=0;
  int mode=0;
  int fd=disastrOS_openResource(disastrOS_getpid(),type,mode);
  printf("fd=%d\n", fd);
  printf("PID: %d, terminating\n", disastrOS_getpid());

  printf("\n-------------Apertura dei semafori-------------\n\n");
  //necessari per il modello produttore - consumatore
  int filled_sem = disastrOS_semOpen(CONSUMER_ID, 0);
  int empty_sem = disastrOS_semOpen(PRODUCER_ID, BUFFER_LENGTH);
  int read_sem = disastrOS_semOpen(READ_ID, 1);
  int write_sem = disastrOS_semOpen(WRITE_ID, 1);

  for (int i = 0; i<ITERATION; ++i){
    if(disastrOS_getpid()%2==0) {  //Se il pid del figlio è pari allora viene eseguita un'operazione di scrittura nel buffer
      producer_function(filled_sem, empty_sem, read_sem, write_sem);
    }
    //Se il pid del figlio è dispari allora viene eseguita un'operazione di lettura sul buffer
    else {
      consumer_function(filled_sem, empty_sem, read_sem, write_sem);
    }
  }

  printf("\n-------------Chiusura dei semafori-------------\n");
  disastrOS_semClose(empty_sem);
  disastrOS_semClose(filled_sem);
  disastrOS_semClose(read_sem);
  disastrOS_semClose(write_sem);

  printf("\n----------Stampo il buffer circolare-----------\n\n");
  int i;
  for(i = 0; i<BUFFER_LENGTH; i++) {
    printf("%d ", buffer[i]);
  }
  printf("\n\n");

  disastrOS_exit(disastrOS_getpid()+1);
}


void initFunction(void* args) {
  disastrOS_printStatus();
  printf("hello, I am init and I just started\n");
  disastrOS_spawn(sleeperFunction, 0);


  printf("I feel like to spawn 10 nice threads\n");
  int alive_children=0;
  for (int i=0; i<10; ++i) {
    int type=0;
    int mode=DSOS_CREATE;
    printf("mode: %d\n", mode);
    printf("opening resource (and creating if necessary)\n");
    int fd=disastrOS_openResource(i,type,mode);
    printf("fd=%d\n", fd);
    disastrOS_spawn(childFunction, 0);
    alive_children++;
  }

  disastrOS_printStatus();
  int retval;
  int pid;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){
    disastrOS_printStatus();
    printf("initFunction, child: %d terminated, retval:%d, alive: %d \n",
	   pid, retval, alive_children);
    --alive_children;
  }
  printf("shutdown!\n");
  disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue
  printf("the function pointer is: %p", childFunction);
  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
