#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

#include "disastrOS_constants.h"

void internal_semPost(){
  int sem_fd = running->syscall_args[0];

  // Get the semaphore descriptor from the file descriptor
  SemDescriptor* sem_desc = (SemDescriptor*) SemDescriptorList_byFd(&(running->sem_descriptors), sem_fd);
  if (!sem_desc) {
    printf("ERROR: Can't find the SemDescriptor\n");
    running->syscall_retvalue = DSOS_ERESOURCENOFD;
    return;
  }

  // Get the sempahore from it's descriptor
  Semaphore* sem = sem_desc->semaphore;
  if (!sem) {
    printf("ERROR: Can't find semaphore\n");
    running->syscall_retvalue = DSOS_ERESOURCENOFD;
    return;
  }

  SemDescriptorPtr* first_sem_wait_desc_ptr;
  
  sem->count++;
  // If the semaphore count is <= 0 and some other thread is waiting, then resume thread
  if (sem->count <= 0 && sem->waiting_descriptors.first != NULL) {
    
    // Get the descriptor of the first process in the list of waiting descriptors
    first_sem_wait_desc_ptr = (SemDescriptorPtr*) List_detach(&(sem->waiting_descriptors),
                                                              (ListItem*) (sem->waiting_descriptors).first);
    
    // Get the pcb of the next process that will run
    PCB* next_pcb = first_sem_wait_desc_ptr->descriptor->pcb;

    // Removing him from waiting list and insert in ready list
    List_detach(&waiting_list, (ListItem*) next_pcb);
    List_insert(&ready_list, ready_list.last, (ListItem*) next_pcb);

    // Set its status on Ready
    next_pcb->status=Ready; 
  }

  running->syscall_retvalue = 0;  // Return 0 on success 
  return;
}