#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"

void internal_semOpen(){
  // do stuff :)
  int sem_id = running->syscall_args[0];  //assegnazione ID del Semaphore
  int sem_count = running->syscall_args[1];  //assegnazione contatore del Semaphore

  //Semaphore* SemaphoreList_byId(SemaphoreList* l, int id);
  Semaphore* sem_aux = SemaphoreList_byId(&semaphores_list, sem_id);  //controllo se il Semaphore con id: sem_id è nella global list of semaphores: semaphores_list, ovvero se è preesistente

  if(sem_aux == NULL) {  //analizzo il caso in cui il Semaphore non esiste
    printf("Creazione del semaforo con id: %d\n", sem_id);
    //Semaphore* Semaphore_alloc(int id, int count);
    sem_aux = Semaphore_alloc(sem_id, sem_count);  //alloco il Semaphore sem_aux avente id: sem_id e contatore inizializzato al valore: sem_count
    //ListItem* List_insert(ListHead* head, ListItem* previous, ListItem* item);
    List_insert(&semaphores_list, semaphores_list.last, (ListItem *)sem_aux);  //aggiorno la lista dei semafori aggiungendo in coda il Semaphore sem_aux sopra creato
  }

  int fd = running->last_sem_fd;  //assegnazione del descrittore all'ultimo semaforo creato
  printf("Creazione del descriptor: %d per il semaforo con id: %d\n", fd, sem_id);
  //SemDescriptor* SemDescriptor_alloc(int fd, Semaphore* res, PCB* pcb);
  SemDescriptor* fd_sem = SemDescriptor_alloc(fd, sem_aux, running);  //alloco il descrittore al semaforo appena creato: sem_aux
  running->last_sem_fd ++;  //incremento il file descriptor del processo running in esecuzione
  List_insert(&running->sem_descriptors, running->sem_descriptors.last, (ListItem *)fd_sem);  //aggiorno la lista dei descrittori aggiungendo in coda il descrittore fd_sem precedentemente creato

  //SemDescriptorPtr* SemDescriptorPtr_alloc(SemDescriptor* descriptor)
  SemDescriptorPtr* fd_semPtr = SemDescriptorPtr_alloc(fd_sem);  //alloco il puntatore al descrittore: fd_semPtr
  fd_sem->ptr = fd_semPtr;  //inserisco il puntatore a descrittore nella Struct del descrittore
  List_insert(&sem_aux->descriptors, sem_aux->descriptors.last, (ListItem *)fd_semPtr);  //aggiungo il puntatore a descittore: fd_semPtr nella lista dei descittori del Semaphore: sem_aux
  running->syscall_retvalue = fd_sem->fd;  //ritorno il file descriptor del Semaphore
}
