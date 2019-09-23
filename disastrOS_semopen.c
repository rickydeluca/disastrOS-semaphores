#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"
#include "disastrOS_constants.h"

void internal_semOpen() {
  
  // Get the id and the count value of the semaphore
  int sem_id = running->syscall_args[0];
  int sem_count = running->syscall_args[1];

  printf("Try to open sempahore with ID: %d ", sem_id);
  printf("and count: %d\n", sem_count);

  // Check if there is already an opened semaphore with te same ID
  Semaphore* sem = SemaphoreList_byId(&semaphores_list, sem_id);

  if (sem) {
      printf("There is already an opened semaphore with the ID: %d\n", sem_id);
      running->syscall_retvalue = DSOS_ERESOURCEOPEN;    
      return;
    }

  // Allocate new semaphore and add it to the global list
  printf("Allocating new semaphore with ID: %d\n", sem_id);
  
  sem = Semaphore_alloc(sem_id, sem_count);
  if (!sem) {                                               // Check if the semaphore was allocated with no problems
    printf("ERROR: Semaphore allocation!\n");
    running->syscall_retvalue = DSOS_ERESOURCECREATE;
    return;
  }

  printf("Semaphore allocation completed\n");

  // Add created semaphore to global list
  List_insert(&semaphores_list, semaphores_list.last, (ListItem*)sem);

  // Create the semaphore descritor
  printf("Allocating SemDescriptor\n");

  SemDescriptor* sem_fd = SemDescriptor_alloc(running->last_sem_fd, sem, running);
  if (!sem_fd) {
     printf("ERROR: SemDescriptor allocation!\n");
     running->syscall_retvalue = DSOS_ERESOURCECREATE;
     return;
  }

  printf("SemDescriptor allocation completed\n");

  // Increment last_sem_fd number so the next semaphore can take a different file descriptor.
  // It also represents the number of opened semaphores for this process
  running->last_sem_fd++;

  // Insert the new descriptor into the process descriptor list
  List_insert(&running->sem_descriptors, running->sem_descriptors.last, (ListItem*) sem_fd);

  printf("Allocating SemDescriptorPtr\n");

  // Create the pointer to the file descriptor
  SemDescriptorPtr* sem_fd_ptr = SemDescriptorPtr_alloc(sem_fd);
  if(!sem_fd_ptr) {
      printf("ERROR: SemDescriptorPtr allocation!\n");
      running->syscall_retvalue = DSOS_ERESOURCECREATE;
      return;
  }

  printf("SemDescriptorPtr allocation completed!\n");

  // Add the descriptor to the semaphore created
  printf("Adding descriptor to the semaphore struct... ");

  sem_fd->ptr = sem_fd_ptr;
  List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*) sem_fd_ptr);

  printf("Done!\n");
  printf("\n \n");

  // Return the sem descriptor to the process
  running->syscall_retvalue = sem_fd->fd;
}
