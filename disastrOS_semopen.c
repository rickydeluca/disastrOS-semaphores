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

  disastrOS_debug("Try to open sempahore with ID: %d ", sem_id);
  disastrOS_debug("and count: %d\n", sem_count);

  // Check if there is already an opened semaphore with the same ID.
  // If YES then I will reuse it. In this way different process can use
  // the same semaphore.
  Semaphore* sem = SemaphoreList_byId(&semaphores_list, sem_id);

  // If not I have to allocate it
  if (!sem) {
      disastrOS_debug("There isn't already an opened semaphore with the ID: %d\n. So I allocate it", sem_id);
      // Allocate new semaphore
      disastrOS_debug("Allocating new semaphore with ID: %d\n", sem_id);
      
      sem = Semaphore_alloc(sem_id, sem_count);
      if (!sem) {                                               // Check if the semaphore was allocated with no problems
        printf("ERROR: Semaphore allocation!\n");
        running->syscall_retvalue = DSOS_ERESOURCECREATE;
        return;
      }

      disastrOS_debug("Semaphore allocation completed\n");

      // Add created semaphore to global list
      List_insert(&semaphores_list, semaphores_list.last, (ListItem*)sem);
    }

  
  // Create the semaphore descritor
  disastrOS_debug("Allocating SemDescriptor\n");

  SemDescriptor* sem_desc = SemDescriptor_alloc(running->last_sem_fd, sem, running);
  if (!sem_desc) {
     printf("ERROR: SemDescriptor allocation!\n");
     running->syscall_retvalue = DSOS_ERESOURCECREATE;
     return;
  }

  disastrOS_debug("SemDescriptor allocation completed\n");

  // Increment last_sem_fd number so the next semaphore can take a different file descriptor.
  // It also represents the number of opened semaphores for this process
  running->last_sem_fd++;

  disastrOS_debug("Allocating SemDescriptorPtr\n");

  // Create the pointer to the file descriptor
  SemDescriptorPtr* sem_desc_ptr = SemDescriptorPtr_alloc(sem_desc);
  if(!sem_desc_ptr) {
      printf("ERROR: SemDescriptorPtr allocation!\n");
      running->syscall_retvalue = DSOS_ERESOURCECREATE;
      return;
  }

  disastrOS_debug("SemDescriptorPtr allocation completed!\n");

  // Add the pointer in the filed of the descriptos
  sem_desc->ptr = sem_desc_ptr;

  // Create a pointer exclusively for when i need to put se process in wait,
  // otherwise there will be a lot of segmentation fault errors.
  SemDescriptorPtr * wait_ptr = SemDescriptorPtr_alloc(sem_desc);  
  if(!wait_ptr) {
      printf("ERROR: wait_ptr allocation!\n");
      running->syscall_retvalue = DSOS_ERESOURCECREATE;
      return;
  }

  sem_desc->wait_ptr = wait_ptr;

  // Inert the descriptor in process' descriptors list
  List_insert(&running->sem_descriptors, running->sem_descriptors.last, (ListItem*) sem_desc);

  // Inserte the descriptor pointer in the semaphore's descriptors list
  List_insert(&sem->descriptors, sem->descriptors.last, (ListItem*) sem_desc_ptr);

  disastrOS_debug("Done!\n");
  disastrOS_debug("\n \n");

  // Return the sem descriptor to the process
  running->syscall_retvalue = sem_desc->fd;
}
