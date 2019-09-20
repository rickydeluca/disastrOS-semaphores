#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semOpen() {

  // Get the id and the count value of the semaphore
  int sem_id = running->syscalls_args[0];
  int sem_count = running->syscalls_args[1];

  printf("Try to open sempahore with ID: %d\n", sem_id);

  // Check if there is already an opened semaphore with te same ID
  Semaphore* sem = SemaphoreList_byId(&semaphores_list, sem_id);

  if (sem) {
      running->syscall_retvalue = DSOS_ESEMAPHORECREATE;    // The error says that there was already an opened semaphore with the same ID
      return;
    }

  // Allocate new semaphore and add it to the global list
  printf("Allocating semaphore with ID: %d\n", sem_id);
  sem = Semaphore_alloc(sem_id, sem_count);
  
  // Add created semaphore to global list
  List_insert(&semaphores_list, semaphores_list.last, (ListItem*)sem);
}
