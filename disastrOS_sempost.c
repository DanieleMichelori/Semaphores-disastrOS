#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_constants.h"

void internal_semPost(){
  // do stuff :)
  int fd = running->syscall_args[0];  //assegnazione file descriptor

  //Descriptor*  DescriptorList_byFd(ListHead* l, int fd)
  SemDescriptor* fd_sem = (SemDescriptor*)SemDescriptorList_byFd(&running->sem_descriptors, fd);  //verifico la presenza del descrittore nella lista dei descittori del semaforo

  if(!fd_sem) {  //caso in cui il descrittore non è presente nella lista
    printf("[ERROR]: Il file descriptor cercato %d non esiste\n", fd);
    running->syscall_retvalue = DSOS_ESEM_FDES_LIST;
    return;
  }
  Semaphore* sem = fd_sem->semaphore;  //risalgo al semaforo del descrittore: fd_sem

  printf("[POST] Il processo %d ha associato il semaforo ID: %d il cui contatore viene incrementato a %d\n", disastrOS_getpid(), sem->id, sem->count+1);
  sem->count = sem->count + 1;  //decremento contatore Semaphore associato alla Wait

  //Sposto il puntatore a descrittore, dalla waiting_list, nella lista di Ready
  if(sem->count <= 0) {  //verifico che il contatore del semaforo abbia valore negativo
    SemDescriptorPtr* auxPtr = (SemDescriptorPtr*)List_detach(&sem->waiting_descriptors, (ListItem *)sem->waiting_descriptors.first);  //rimuovo il primo puntatore a descrittore dalla waiting_list del semaforo

    if(!auxPtr) {  //caso in cui la rimozione del puntatore a descrittore, dalla waiting list, fallisce
      printf("[ERROR]: Rimozione del puntatore a descrittore, dalla waiting_list, fallita!\n");
      running->syscall_retvalue = DSOS_ELIST_DETACH;
      return;
    }

    SemDescriptorPtr* ret0 = (SemDescriptorPtr*)List_insert(&sem->descriptors, sem->descriptors.last, (ListItem *)auxPtr);  //inserisco il puntatore a descrittore precedentemente rimosso: auxPtr nella lista dei descrittori del semaforo (in coda)

    if(!ret0) {  //caso in cui l'inserimento del puntatore a descrittore, nella lista, fallisce
      printf("[ERROR]: Inserimento del puntatore a descrittore fallita!\n");
      running->syscall_retvalue = DSOS_ELIST_INSERT;
      return;
    }

    //imposto lo stato del processo corrente
    PCB* proc = auxPtr->descriptor->pcb;  /*(SemDescriptorPtr->SemDescriptor->PCB)*/
    proc->status = Ready;

    //sposto il PCB dalla waiting alla ready list
    PCB* pcb_aux = (PCB*)List_detach(&waiting_list, (ListItem *)proc);  //rimuovo il process control block del processo corrente dalla waiting_list

    if(!pcb_aux) {
      printf("[ERROR]: Rimozione del processo, dalla waiting_list, fallita!\n");
      running->syscall_retvalue= DSOS_ELIST_DETACH;
      return;
    }
    pcb_aux = (PCB*)List_insert(&ready_list, ready_list.last, (ListItem *)proc);  //inserisco il process control block del processo corrente nella ready_list (in coda)

    if(!pcb_aux) {
      printf("[ERROR]: Inserimento del processo, nella ready_list, fallita!\n");
      running->return_value = DSOS_ELIST_INSERT;
      return;
    }
  }

  running->syscall_retvalue = 0;
}
