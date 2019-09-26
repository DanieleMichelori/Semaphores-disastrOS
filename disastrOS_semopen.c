#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"
#include "disastrOS_constants.h"

void internal_semOpen(){
  // do stuff :)
  int sem_id = running->syscall_args[0];  //assegnazione ID del Semaphore
  int sem_count = running->syscall_args[1];  //assegnazione contatore del Semaphore

  //Semaphore* SemaphoreList_byId(SemaphoreList* l, int id);
  Semaphore* sem_aux = (Semaphore*)SemaphoreList_byId(&semaphores_list, sem_id);  //controllo se il Semaphore con id: sem_id è nella global list of semaphores: semaphores_list, ovvero se è preesistente

  if(sem_id < 0) {  //caso in cui l'id è negativo
    printf("[ERROR]: id semaforo non valido\n");
    running->syscall_retvalue = DSOS_EWRONG_ID;
    return;
  }

  if(sem_aux == NULL) {  //analizzo il caso in cui il Semaphore non esiste
    printf("Creazione del semaforo con id: %d...\n", sem_id);
    //Semaphore* Semaphore_alloc(int id, int count);
    sem_aux = (Semaphore*)Semaphore_alloc(sem_id, sem_count);  //alloco il Semaphore sem_aux avente id: sem_id e contatore inizializzato al valore: sem_count

    if(!sem_aux) {  //caso in cui l'allocazione del semaforo fallisce
      printf("[ERROR]: allocazione del semaforo con id: %d fallita!\n", sem_id);
      running->syscall_retvalue = DSOS_ESEM_ALLOC;
      return;
    }

    //ListItem* List_insert(ListHead* head, ListItem* previous, ListItem* item);
    SemDescriptor* ret0 = (SemDescriptor*)List_insert(&semaphores_list, semaphores_list.last, (ListItem *)sem_aux);  //aggiorno la lista dei semafori aggiungendo in coda il Semaphore sem_aux sopra creato

    if(!ret0) {  //caso in cui l'inserimento del semaforo, nella lista, fallisce
      printf("[ERROR]: inserimento del semaforo in coda fallita!\n");
      running->syscall_retvalue = DSOS_ELIST_INSERT;
      return;
    }
  }

  int fd = running->last_sem_fd;  //assegnazione del descrittore all'ultimo semaforo creato
  printf("Creazione del descriptor: %d per il semaforo con id: %d...\n\n", fd, sem_id);
  //SemDescriptor* SemDescriptor_alloc(int fd, Semaphore* res, PCB* pcb);
  SemDescriptor* fd_sem = (SemDescriptor*)SemDescriptor_alloc(fd, sem_aux, running);  //alloco il descrittore al semaforo appena creato: sem_aux

  if(!fd_sem) {  //caso in cui l'allocazione del descrittore fallisce
    printf("[ERROR]: allocazione del descrittore: %d fallita!\n", fd);
    running->syscall_retvalue = DSOS_ESEM_FDES_ALLOC;
    return;
  }

  running->last_sem_fd ++;  //incremento il file descriptor del processo running in esecuzione
  SemDescriptor* ret1 = (SemDescriptor*)List_insert(&running->sem_descriptors, running->sem_descriptors.last, (ListItem *)fd_sem);  //aggiorno la lista dei descrittori aggiungendo in coda il descrittore fd_sem precedentemente creato

  if(!ret1) {  //caso in cui l'inserimento del descrittore, nella lista, fallisce
    printf("[ERROR]: inserimento del descrittore in coda fallita!\n");
    running->syscall_retvalue = DSOS_ELIST_INSERT;
    return;
  }

  //SemDescriptorPtr* SemDescriptorPtr_alloc(SemDescriptor* descriptor)
  SemDescriptorPtr* fd_semPtr = (SemDescriptorPtr*)SemDescriptorPtr_alloc(fd_sem);  //alloco il puntatore al descrittore: fd_semPtr

  if(!fd_semPtr) {  //caso in cui l'allocazione del puntatore a descittore fallisce
    printf("[ERROR]: allocazione del puntatore a descrittore fallita!\n");
    running->syscall_retvalue = DSOS_ESEM_FDES_PTR_ALLOC;
    return;
  }

  fd_sem->ptr = fd_semPtr;  //inserisco il puntatore a descrittore nella Struct del descrittore
  SemDescriptorPtr* ret2 = (SemDescriptorPtr*)List_insert(&sem_aux->descriptors, sem_aux->descriptors.last, (ListItem *)fd_semPtr);  //aggiungo il puntatore a descittore: fd_semPtr nella lista dei descittori del Semaphore: sem_aux

  if(!ret2) {  //caso in cui l'inserimento del puntatore a descittore, nella lista, fallisce
    printf("[ERROR]: inserimento del puntatore a descrittore in coda fallita!\n");
    running->syscall_retvalue = DSOS_ELIST_INSERT;
    return;
  }

  running->syscall_retvalue = fd_sem->fd;  //ritorno il file descriptor del Semaphore
}
