#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semClose(){
  // do stuff :)
  int fd = running->syscall_args[0];  //assegnazione file descriptor processo corrente

  //Descriptor*  DescriptorList_byFd(ListHead* l, int fd)
  SemDescriptor* fd_sem = SemDescriptorList_byFd(&running->sem_descriptors, fd);  //verifico che il descrittore fd si trovi nella lista dei descrittori del semaforo

  if(!fd_sem) {  //caso in cui il descittore fd non è presente
    printf("ERROR: il file descriptor cercato: %d non è presente\n", fd);
    return;
  }
  printf("Il file descriptor cercato: %d esiste!\n", fd);

  printf("Rimozione del descriptor: %d associato al semaforo con id: %d\n\n", fd, fd_sem->semaphore->id);
  //ListItem* List_detach(ListHead* head, ListItem* item);
  fd_sem = List_detach(&running->sem_descriptors, fd_sem);  //rimuovo il fd dalla lista dei descrittori del processo
}
