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

  int sem_fd = running->syscall_args[0];
  
  disastrOS_debug("Obtaining the semaphore descriptor... ");

  // Get the SemDescriptor of the semaphore to close from SemDescrpitorList of the process
  SemDescriptor* sem_desc =  SemDescriptorList_byFd(&running->sem_descriptors, sem_fd);
  if (!sem_desc) {
    printf("ERROR: SemDescriptor not found in the process\n");
    running->syscall_retvalue = DSOS_ERESOURCENOFD;
    return;
  }

  disastrOS_debug("Done!\n");

  disastrOS_debug("Obtaining the semaphore... ");

  // Obtain the semaphore from its file descriptor
  Semaphore* sem = sem_desc->semaphore;
  if (!sem) {
    printf("ERROR: Semaphore not found in the process\n");
    running->syscall_retvalue = DSOS_ERESOURCENOFD;
    return;
  }

  disastrOS_debug("Done!\n");

  disastrOS_debug("Obtainign pointer to SemDescriptor... ");

  // Obatin pointer to SemDescriptor
  SemDescriptorPtr* sem_desc_ptr = sem_desc->ptr;
  if (!sem_desc_ptr) {
    printf("ERROR: Can't find the SemDescriptorPointer\n");
    running->syscall_retvalue = DSOS_ERESOURCENOFD;
    return;
  }

  SemDescriptorPtr* sem_desc_wait_ptr = sem_desc->wait_ptr;

  disastrOS_debug("Done!\n");

  /*  N.B. The return value of functions "*_free" is a type called PoolAllocatorResult
      and defined in "pool_allcator.h". It values:
      0x0 -> Success
      -1  -> NotEnoughMemory
      -2  -> UnalignedFree
      -3  -> OutOfRange
      -4  -> DoubleFree
  */

  int ret = 0;  // Return value of functions "*_free"

  disastrOS_debug("Removing SemDescriptor... ");

  // Remove SemDescrpitor from the list of the process
  sem_desc = (SemDescriptor*) List_detach( &(running->sem_descriptors), (ListItem*) sem_desc);
  ret = SemDescriptor_free(sem_desc);
  if (ret < 0) {
    printf("ERROR: Can't free SemDescriptor\n");
    running->syscall_retvalue = ret;
    return;
  }

  disastrOS_debug("Done!\n");

  disastrOS_debug("Removing SemDescriptorPtr... ");

  // Remove SemDescriptorPtr from the list of the process
  sem_desc_ptr = (SemDescriptorPtr*) List_detach( &(sem->descriptors), (ListItem*) sem_desc_ptr);
  ret = SemDescriptorPtr_free (sem_desc_ptr);
  if (ret < 0) {
    printf("ERROR: Can't free SemDescriptorPtr\n");
    running->syscall_retvalue = ret;
    return;
  }

  ret = SemDescriptorPtr_free(sem_desc_wait_ptr);
  if (ret < 0) {
    printf("ERROR: Can't free sem_desc_wait_ptr\n");
    running->syscall_retvalue = ret;
    return;
  }

  // Check if I detached all the descriptors. If true no process is still using the semaphore so I can unlink it
  int check_size_descriptors_list = sem->descriptors.size;

  if (check_size_descriptors_list > 0) {  // if > 0 then some other process is using the semaphore
    running->syscall_retvalue = 0;
  } else {
    sem = (Semaphore*) List_detach(&semaphores_list, (ListItem*) sem);
    ret = Semaphore_free(sem);
    if (ret < 0) {
      printf("ERROR: Can't free the semaphore\n");
      running->syscall_retvalue = ret;
      return;
    }

  // On success return 0
  running->syscall_retvalue = 0;
  }

}
