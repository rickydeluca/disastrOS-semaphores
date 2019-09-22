#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

// Include these libs to use the error code and other variables useful (like semaphore_list)
#include "disastrOS_globals.h"
#include "disastrOS_constants.h"

void internal_semClose(){
  
  // Get the SemDescriptor of the semaphore to close from SemDescrpitorList of the process
  int sem_fd = running->syscall_args[0];

  SemDescriptor* sem_desc =  SemDescriptorList_byFd(&running->sem_descriptors, sem_fd);
  if (!sem_desc) {
    printf("ERROR: SemDescriptor not found in the process\n");
    running->syscall_retvalue = DSOS_ERESOURCENOFD;
    return;
  }

  // On success return 0
  running->syscall_retvalue = 0;
}
