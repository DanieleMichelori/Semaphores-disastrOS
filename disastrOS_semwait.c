#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semWait(){
  // do stuff :)
  int fd = running->syscall_args[0];  //assegnazione file descriptor
  //Descriptor*  DescriptorList_byFd(ListHead* l, int fd)
  SemDescriptor* fd_sem = SemDescriptorList_byFd(&running->sem_descriptors, fd);  //verifico la presenza del descrittore nella lista dei descittori del semaforo
  Semaphore* sem = fd_sem->semaphore;  //risalgo al semaforo del descrittore: fd_sem

  if(sem->count >= 0) {
    printf("Il contatore del semaforo con id: %d ha valore: %d\n", sem->id, sem->count);
  }

  sem->count = sem->count - 1;  //decremento contatore Semaphore associato alla Wait

  if(sem->count < 0) {  //il contatore del semaforo ha valore negativo -> sospendo il task
    printf("Il contatore del semaforo con id: %d ha valore negativo: %d\n", sem->id, sem->count);
  }


}
