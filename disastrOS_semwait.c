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

  printf("[WAIT] Il contatore del semaforo con id: %d passa dal valore: %d al valore: %d\n", sem->id, sem->count, sem->count-1);
  sem->count = sem->count - 1;  //decremento contatore Semaphore associato alla Wait
  SemDescriptorPtr* fd_semPtr = fd_sem->ptr;

  printf("Sposto il puntatore a descrittore nella lista di Waiting...\n");
  if(sem->count < 0) {  //il contatore del semaforo ha valore negativo -> sospendo il task
    SemDescriptorPtr* ret = List_detach(&sem->descriptors, (ListItem *) fd_semPtr);  //rimuovo il puntatore a descrittore dalla lista dei descittori del semaforo
    ret = List_insert(&sem->waiting_descriptors, sem->waiting_descriptors.last, (ListItem *) fd_semPtr);  //inserisco il puntatore a descrittore nella lista di waiting del semaforo
  }





}
