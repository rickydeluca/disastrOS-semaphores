#include    <stdio.h>
#include    <stdlib.h>

#include    "disastrOS.h"
#include    "disastrOS_semaphore.h"
#include    "disastrOS_semdescriptor.h"
#include    "disastrOS_constants.h"
#include    "disastrOS_globals.h"

#define     EXIT_FAILURE        1
#define     EXIT_SUCCESSFUL     0

/***************************** SEMOPEN TEST *****************************/
void semopen_test(void* args) {
    int sem_id  = 1;
    int count   = 1;
    if (disastrOS_semOpen(sem_id, count) < 0) {
        printf("TEST ERROR: semOpen\n");
        exit(EXIT_FAILURE);
    }

    printf("TEST SUCCESSFUL: semOpen\n");
    exit(EXIT_SUCCESSFUL);

}
/*************************************************************************/

/***************************** SEMCLOSE TEST *****************************/
void semclose_test(void* args) {
    
    int sem_id  = 1;
    int count   = 1;

    int sem_fd  = 0;

    sem_fd = disastrOS_semOpen(sem_id, count);
    if (sem_fd < 0) {
        printf("TEST ERROR: semOpen\n");
        exit(EXIT_FAILURE);
    }

    //printf(" Done!\n");

    printf ("Closing the semaphore with ID: %d\n", sem_id);

    if (disastrOS_semClose(sem_fd) != 0) {
        printf("TEST ERROR: semClose\n");
        exit(EXIT_FAILURE);
    }

    //printf(" Done!\n");

    printf("TEST SUCCESSFUL: semClose\n");
    exit(EXIT_SUCCESSFUL);
}
/*************************************************************************/


/***************************** SEMWAIT TEST ******************************/
void semwait_test_2(void* args) {
    int sem_id      = *((int*)args);
    int sem_count   = 30;

    int sem_fd = 0;
    sem_fd = disastrOS_semOpen(sem_id, sem_count);

    SemDescriptor* sem_desc = (SemDescriptor*) SemDescriptorList_byFd(&(running->sem_descriptors), sem_fd );

    printf("Try testing semWait with 31 iterations\n");

    int i;
    for (i = 0; i < 31; i++) {
        printf("Iteration number: \t %d\n", i);
        printf("Sem count value: \t %d\n", sem_desc->semaphore->count);
        if(disastrOS_semWait(sem_fd) != 0) {
            printf("TEST ERROR: semWait\n");
            exit(EXIT_FAILURE);
        }
    }

    printf("TEST SUCCESS: semWait\n");
    exit(EXIT_SUCCESS);
}

void semwait_test(void* args) {
    int sem_id      = *((int*)args);
    int sem_count   = 30;

    int sem_fd = 0;
    sem_fd = disastrOS_semOpen(sem_id, sem_count);

    // Spanning a new process
    sem_id++;
    disastrOS_spawn(semwait_test_2, &sem_id);

    SemDescriptor* sem_desc = (SemDescriptor*) SemDescriptorList_byFd(&(running->sem_descriptors), sem_fd );

    printf("Try testing semWait with 31 iterations\n");

    int i;
    for (i = 0; i < 31; i++) {
        printf("Iteration number: \t %d\n", i);
        printf("Sem count value: \t %d\n", sem_desc->semaphore->count);
        if(disastrOS_semWait(sem_fd) != 0) {
            printf("TEST ERROR: semWait\n");
            exit(EXIT_FAILURE);
        }
    }

    printf("TEST SUCCESS: semWait\n");
    exit(EXIT_SUCCESS);
}
/*************************************************************************/


/***************************** SEMPOST TEST ******************************/
int num_iter = 30;

typedef struct sempostTest_data {
    int* buff;
} sempostTest_data;

void sempostTest_consumer(void* args) {
    

    int i;
    for (i = 0; i < num_iter; i++) {
        disastrOS_semWait();
        disastrOS_semWait()
    }
}

void sempostTest_producer(void* args) {

}

void sempost_test(void* args) {
    int full_sem_id  = 0;
    int empty_sem_id = 1;
    int mutex_sem_id = 2;

    // Allocate the buffer that is the shared variable
    int buff_size = 10;
    int buff[buff_size];

    // Allcate args to pass to the threads
    sempostTest_data data;

    // Populate the buffer
    int i;
    for (i = 0; i < 10; i++) {
        buff[i] = 1;
    }   // I should have at this point: buff = {1 1 1 1 1 1 1 1 1 1}

    // Open the semaphores in the main process so I can close them later
    int full_sem_fd  = disastrOS_semOpen(full_sem_id, 0);
    int empty_sem_fd = disastrOS_semOpen(empty_sem_id, buff_size);
    int mutex_sem_fd = disastrOS_semOpen(mutex_sem_id, 1);

    // Lanch a consumer and a producert
    disastrOS_spawn(sempostTest_consumer, buff);
    disastrOS_spawn(sempostTest_producer, buff);

    // Close semaphores opend previously
    disastrOS_semClose(full_sem_fd);
    disastrOS_semClose(empty_sem_fd);
    disastrOS_semClose(mutex_sem_fd);

    printf("TEST SUCCESSFUL: semPost\n");
    exit(EXIT_SUCCESS);
}
/*************************************************************************/


int main(int argc, char** argv) {
    int test_num;
    
    char* logfilename=0;
    if (argc>1) {
        logfilename = argv[1];
    }
   
    // Single function tests
    printf("What would you like to test?\n    1 - semOpen\n    2 - semClose\n    3 - semWait\n");

    scanf("%d", &test_num);
    if (test_num == 1) {
        printf("Testing semOpen\n");
        disastrOS_start(semopen_test, 0, logfilename);
    } else if (test_num == 2) {
        printf("Testing semClose\n");
        disastrOS_start(semclose_test, 0, logfilename);
    } else if (test_num == 3) {
        int sem_id = 0;
        printf("Testing semWait\n");
        disastrOS_start(semwait_test, &sem_id, logfilename);
    }else {
        printf("Wrong input. Sayonara!");
    }

    return 0;
}