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
  SemDescriptor* fd_sem = (SemDescriptor*)SemDescriptorList_byFd(&running->sem_descriptors, fd);  //verifico che il descrittore: fd si trovi nella lista dei descrittori del semaforo

  if(!fd_sem) {  //caso in cui il descittore fd non è presente nella lista dei descrittori del semaforo
    printf("[ERROR]: il file descriptor cercato: %d non è presente\n", fd);
    return;
  }
  printf("Il file descriptor cercato: %d esiste!\n", fd);

  printf("Rimozione del descriptor: %d associato al semaforo con id: %d...\n", fd, fd_sem->semaphore->id);
  //ListItem* List_detach(ListHead* head, ListItem* item);
  fd_sem = (SemDescriptor*)List_detach(&running->sem_descriptors, (ListItem *)fd_sem);  //rimuovo il fd dalla lista dei descrittori del processo

  if(!fd_sem) {  //caso in cui la rimozione del descrittore, dalla lista, fallisce
    printf("[ERROR]: rimozione del descrittore %d fallita!\n", fd);
    running->syscall_retvalue = DSOS_ELIST_DETACH;
    return;
  }

  int ret0 = SemDescriptor_free(fd_sem);  //dealloco la memoria occupata dal descriptor: fd

  //Success == 0 in PoolAllocator_releaseBlock associata a: SemDescriptor_free
  if(ret0 != 0) {  //caso in cui la deallocazione del descrittore fallisce
    printf("[ERROR]: deallocazione della memoria associata al descrittore %d fallita!\n", fd);
    running->syscall_retvalue = DSOS_ESEM_FDES_FREE;
    return;
  }

  Semaphore* sem = fd_sem->semaphore;  //ottengo il semaforo: sem a partire dal descrittore

  printf("Rimozione del puntatore al descriptor associato al semaforo con id: %d...\n", fd_sem->semaphore->id);
  SemDescriptorPtr* fd_semPtr = (SemDescriptorPtr*)List_detach(&sem->descriptors, (ListItem*)fd_sem->ptr);  //risalgo e rimuovo il puntatore a descrittore: fd_sem->ptr

  if(!fd_semPtr) {  //caso in cui la rimozione del puntatore a descrittore, dalla lista, fallisce
    printf("[ERROR]: rimozione del puntatore a descrittore fallita!\n");
    running->syscall_retvalue = DSOS_ELIST_DETACH;
    return;
  }

  int ret1 = SemDescriptorPtr_free(fd_semPtr);  //dealloco la memoria occupata dal puntatore a descrittore

  //Success == 0 in PoolAllocator_releaseBlock associata a: SemDescriptorPtr_free
  if(ret1 != 0) {  //caso in cui la deallocazione del descrittore fallisce
    printf("[ERROR]: deallocazione della memoria associata al puntatore a descrittore fallita!\n");
    running->syscall_retvalue = DSOS_ESEM_FDES_PTR_FREE;
    return;
  }

  running->last_sem_fd--;  //decremento il contatore dei descrittori attivi del processo

  if(sem->descriptors.size == 0) {  //controllo che non ci siano descrittori attivi nel semaforo: sem
    printf("Rimozione del semaforo con id: %d...\n\n", sem->id);
    Semaphore* ret2 = (Semaphore*)List_detach(&semaphores_list, (ListItem *)sem);  //rimuovo il semaforo dalla lista dei semafori

    if(!ret2) {  //caso in cui la rimozione del semaforo, dalla lista, fallisce
      printf("[ERROR]: rimozione del semaforo con id: %d fallita!\n", sem->id);
      running->syscall_retvalue = DSOS_ELIST_DETACH;
      return;
    }

    int ret3 = Semaphore_free(sem);  //dealloco la memoria occupata dal semaforo

    //Success == 0 in PoolAllocator_releaseBlock associata a: Semaphore_free
    if(ret3 != 0) {  //caso in cui la deallocazione del descrittore fallisce
      printf("[ERROR]: deallocazione della memoria associata al semaforo con id: %d fallita!\n", sem->id);
      running->syscall_retvalue = DSOS_ESEM_FREE;
      return;
    }
  }
}
