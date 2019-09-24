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
  SemDescriptor* fd_sem = SemDescriptorList_byFd(&running->sem_descriptors, fd);  //verifico che il descrittore: fd si trovi nella lista dei descrittori del semaforo

  if(!fd_sem) {  //caso in cui il descittore fd non è presente nella lista dei descrittori del semaforo
    printf("ERROR: il file descriptor cercato: %d non è presente\n", fd);
    return;
  }
  printf("Il file descriptor cercato: %d esiste!\n", fd);

  printf("Rimozione del descriptor: %d associato al semaforo con id: %d\n", fd, fd_sem->semaphore->id);
  //ListItem* List_detach(ListHead* head, ListItem* item);
  fd_sem = List_detach(&running->sem_descriptors, (ListItem *)fd_sem);  //rimuovo il fd dalla lista dei descrittori del processo
  SemDescriptor_free(fd_sem);  //dealloco la memoria occupata dal descriptor: fd

  Semaphore* sem = fd_sem->semaphore;  //ottengo il semaforo: sem a partire dal descrittore

  printf("Rimozione del puntatore al descriptor associato al semaforo con id: %d\n", fd_sem->semaphore->id);
  SemDescriptorPtr* fd_semPtr = List_detach(&sem->descriptors, (ListItem*)fd_sem->ptr);  //risalgo e rimuovo il puntatore a descrittore: fd_sem->ptr
  SemDescriptorPtr_free(fd_semPtr);  //dealloco la memoria occupata dal puntatore a descrittore

  running->last_sem_fd--;  //decremento il contatore dei descrittori attivi del processo

  if(sem->descriptors.size == 0) {  //controllo che non ci siano descrittori attivi nel semaforo: sem
    printf("Rimozione del semaforo con id: %d\n\n", sem->id);
    List_detach(&semaphores_list, (ListItem *)sem);  //rimuovo il semaforo dalla lista dei semafori
    Semaphore_free(sem);  //dealloco la memoria occupata dal semaforo
  }
}
