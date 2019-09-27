#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

#include "disastrOS_constants.h"

void internal_semWait() {
  
  int sem_fd = running->syscall_args[0];

  // Get the SemDescriptor from the opened semaphores
  SemDescriptor* sem_desc = (SemDescriptor*) SemDescriptorList_byFd(&(running->sem_descriptors), sem_fd);
  if (!sem_desc) {
    printf("ERROR: SemDescriptor not found in the process\n");
    running->syscall_retvalue = DSOS_ERESOURCENOFD;
    return;
  }
  
  // Get the semaphore from its descriptor
  Semaphore* sem = sem_desc->semaphore;

  // Decrement the semaphore's value of count
  sem->count--;
  running->syscall_retvalue = 0;
  
  // Check the count value. If <= 0 then put the caller on waiting
  if (sem->count < 0) {
    // Put the caller on waiting
    running->status = Waiting;

    List_insert(&waiting_list, waiting_list.last, (ListItem*) running);

    SemDescriptorPtr* sem_desc_ptr = (SemDescriptorPtr*) List_detach(&sem->descriptors,
                                                                    (ListItem*) sem_desc->ptr);
   
    List_insert( &(sem->waiting_descriptors),
                sem->waiting_descriptors.last,
                (ListItem*) sem_desc_ptr);

    List_insert(&waiting_list, waiting_list.last, (ListItem*) sem_desc->pcb);

    /* Start the first process in ready list
    if(ready_list.first == NULL) {
        printf("ERROR: No process in the ready list. \nPossilbe deadlock!\n");
        running->syscall_retvalue = DSOS_EDEADLOCK;
        return;
    } */
    
    running = (PCB*) List_detach(&ready_list, (ListItem*) ready_list.first);
  }

  return;
}
