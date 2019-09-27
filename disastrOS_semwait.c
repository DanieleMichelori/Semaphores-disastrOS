#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_constants.h"

void internal_semWait(){
  // do stuff :)
  int fd = running->syscall_args[0];  //assegnazione file descriptor
  //Descriptor*  DescriptorList_byFd(ListHead* l, int fd)
  SemDescriptor* fd_sem = (SemDescriptor*)SemDescriptorList_byFd(&running->sem_descriptors, fd);  //verifico la presenza del descrittore nella lista dei descittori del semaforo

  if(!fd_sem) {  //caso in cui il descrittore non è presente nella lista
    printf("[ERROR]: il file descriptor cercato: %d non esiste\n", fd);
    running->syscall_retvalue = DSOS_ESEM_FDES_LIST;
    return;
  }
  Semaphore* sem = fd_sem->semaphore;  //risalgo al semaforo del descrittore: fd_sem

  printf("[WAIT] Il contatore del semaforo con id: %d passa dal valore: %d al valore: %d\n", sem->id, sem->count, sem->count-1);
  sem->count = sem->count - 1;  //decremento contatore Semaphore associato alla Wait

  SemDescriptorPtr* fd_semPtr = fd_sem->ptr;  //risalgo al puntatore a descittore

  //Sposto il puntatore a descrittore nella lista di Waiting
  if(sem->count < 0) {  //il contatore del semaforo ha valore negativo -> sospendo il task
    SemDescriptorPtr* ret0 = (SemDescriptorPtr*)List_detach(&sem->descriptors, (ListItem *) fd_semPtr);  //rimuovo il puntatore a descrittore: fd_semPtr dalla lista dei descittori del semaforo

    if(!ret0) {  //caso in cui la rimozione del puntatore a descrittore, dalla lista, fallisce
      printf("[ERROR]: rimozione del puntatore a descrittore fallita!\n");
      running->syscall_retvalue = DSOS_ELIST_DETACH;
      return;
    }

    SemDescriptorPtr* ret1 = (SemDescriptorPtr*)List_insert(&sem->waiting_descriptors, sem->waiting_descriptors.last, (ListItem *) fd_semPtr);  //inserisco il puntatore a descrittore: fd_semPtr nella lista di waiting del semaforo

    if(!ret1) {  //caso in cui l'inserimento del puntatore a descrittore, nella lista di waiting, fallisce
      printf("[ERROR]: inserimento del puntatore a descrittore, nella waiting_list, fallita!\n");
      running->syscall_retvalue = DSOS_ELIST_INSERT;
      return;
    }


  }





}
