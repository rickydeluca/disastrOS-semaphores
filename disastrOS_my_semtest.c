#include    <stdio.h>
#include    <stdlib.h>

#include    "disastrOS.h"
#include    "disastrOS_semaphore.h"
#include    "disastrOS_semdescriptor.h"
#include    "disastrOS_constants.h"
#include    "disastrOS_globals.h"

#define     EXIT_FAILURE        1
#define     EXIT_SUCCESSFUL     0

// Define constant used in the complete tests
#define     FULL_SEM_ID         0
#define     EMPTY_SEM_ID        1
#define     MUTEX_SEM_ID        2

#define     NUM_ITER            1
#define     NUM_CHILDREN        10

#define     BUFFER_SIZE         10

typedef struct shared_buffer {
    int buff[BUFFER_SIZE];
} shared_buffer;

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

    SemDescriptor* sem_desc = (SemDescriptor*) SemDescriptorList_byFd( &(running->sem_descriptors), sem_fd );

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
// Sem id declared globally so child processes can re use the same semaphore opened in the main process
#define full_sem_id   0
#define empty_sem_id  1
#define mutex_sem_id  2

int num_iter = 2000;

int shared_var = 0;

void sempostTest_consumer(void* args) {
    int* sh_var = (int*) args;
    
    //int full_sem  = disastrOS_semOpen(full_sem_id, 0);
    //int empty_sem = disastrOS_semOpen(empty_sem_id, 1);
    int mutex_sem = disastrOS_semOpen(mutex_sem_id, 1);

    /* Debug print to check the fd value
    printf("CONSUMER:\n");
    printf("\t - full_sem_fd: %d\n\t - empty_sem_fd: %d\n\t - mutex_sem_fd: %d\n", full_sem, empty_sem, mutex_sem);
    */

    int i;
    for (i = 0; i < num_iter; i++) {
        // Check on entry
        //printf("CONSUMER: Just before wait\n");
        //disastrOS_semWait(full_sem);
        disastrOS_semWait(mutex_sem);

        // Access from consumer to the shared variable
        (*sh_var)++;
        printf("CONSUMER:\t Shared variable value:\t %d\n\n", shared_var);

        //printf("CONSUMER: Just before post\n");
        disastrOS_semPost(mutex_sem);
        //disastrOS_semPost(empty_sem);
    }

    disastrOS_exit(EXIT_SUCCESS);
}

void sempostTest_producer(void* args) {  
    int* sh_var = (int*) args;

    //int full_sem  = disastrOS_semOpen(full_sem_id, 0);
    //int empty_sem = disastrOS_semOpen(empty_sem_id, 1);
    int mutex_sem = disastrOS_semOpen(mutex_sem_id, 1);

    /* Debug print to check the fd value
    printf("PRODUCER:\n");
    printf("\t - full_sem_fd: %d\n\t - empty_sem_fd: %d\n\t - mutex_sem_fd: %d\n", full_sem, empty_sem, mutex_sem);
    */

    int i;
    for (i = 0; i < num_iter; i++) {
        // Check on entry
        //printf("PRODUCER: Just before wait\n");
        //disastrOS_semWait(empty_sem);
        disastrOS_semWait(mutex_sem);

        // Access from consumer to the shared buffer
        (*sh_var)--;
        printf("PRODUCER:\t Shared variable value:\t %d\n\n", shared_var);

        //printf("PRODUCER: Just before post\n");
        disastrOS_semPost(mutex_sem);
        //disastrOS_semPost(full_sem);
    }
    disastrOS_exit(EXIT_SUCCESS);
}   

void sempost_test(void* args) {
    printf ("Testing the semPost using a simple Producer Consumer model that read and write in a buffer\n");

    // Open the semaphores in the main process so I can close them later
    //int full_sem_fd  = disastrOS_semOpen(full_sem_id, 0);
    //int empty_sem_fd = disastrOS_semOpen(empty_sem_id, 1);
    int mutex_sem_fd = disastrOS_semOpen(mutex_sem_id, 1);

    /* Debug print to check the fd value
    printf("PARENT:\n");
    printf("\t - full_sem_fd: %d\n\t - empty_sem_fd: %d\n\t - mutex_sem_fd: %d\n\n", full_sem_fd, empty_sem_fd, mutex_sem_fd);
    */

    // Initial value of the shared variable
    printf("PARENT:\t Shared variable value:\t %d\n\n", shared_var);
    // Lanch a consumer and a producer
    disastrOS_spawn(sempostTest_consumer, &shared_var);
    disastrOS_spawn(sempostTest_producer, &shared_var);

    // Wait for childs to finish
    disastrOS_wait(0, NULL);
    disastrOS_wait(0, NULL);

    // Close semaphores opend previously
    //disastrOS_semClose(full_sem_fd);
    //disastrOS_semClose(empty_sem_fd);
    disastrOS_semClose(mutex_sem_fd);

    printf("TEST SUCCESSFUL: semPost\n");
    disastrOS_shutdown();
}
/*************************************************************************/

/************************** COMPLETE TEST ********************************/
void disastrOS_consumer(void* args) {
    shared_buffer* sh_buff = (shared_buffer*) args;
    int this_pid = running->pid;
    int i, j;   // Iterators for the CS

    // Re-open the semaphores in the childs. In semOpen if a semaphore is already opened
    // it will not be reallocated, instead re-used. So here I'm using the same semaphores
    // opend in the parent process.
    int full_sem = disastrOS_semOpen(FULL_SEM_ID, 0);
    if (full_sem < 0) {
        printf("CONSUMER [%d] ERROR: disastrOS_semOpen, full_sem\n", this_pid);
        disastrOS_exit(EXIT_FAILURE);
    }

    int empty_sem = disastrOS_semOpen(EMPTY_SEM_ID, BUFFER_SIZE);
    if (empty_sem < 0) {
        printf("CONSUMER [%d] ERROR: disastrOS_semOpen, empty_sem\n", this_pid);
        disastrOS_exit(EXIT_FAILURE);
    }

    int mutex_sem = disastrOS_semOpen(MUTEX_SEM_ID, 1);
    if (mutex_sem < 0) {
        printf("CONSUMER [%d] ERROR: disastrOS_semOpen, mutex_sem\n", this_pid);
        disastrOS_exit(EXIT_FAILURE);
    }

    for (i = 0; i < NUM_ITER; i++) {
       for (j = 0; j < BUFFER_SIZE; j++) {
            if (disastrOS_semWait(full_sem) < 0) {
                printf("CONSUMER [%d] ERROR: disastrOS_semWait full_sem\n", this_pid);
                disastrOS_exit(EXIT_FAILURE);
            }
            if (disastrOS_semWait(mutex_sem) < 0) {
                printf("CONSUMER [%d] ERROR: disastrOS_semWait mutex_sem\n", this_pid);
                disastrOS_exit(EXIT_FAILURE);
            }

            sh_buff->buff[j]--;
            printf("CONSUMER [%d]:\t Shared buffer at j = %d\t values\t %d\n", this_pid, j, sh_buff->buff[j]);

            //sh_buff->buff[BUFFER_SIZE%i]--;
            //printf("CONSUMER [%d]:\t Shared buffer value at i = %d\t values\t %d\n", this_pid, BUFFER_SIZE%i, sh_buff->buff[BUFFER_SIZE%i]);

            if (disastrOS_semPost(mutex_sem) != 0) {
                printf("CONSUMER [%d] ERROR: disastrOS_semPost mutex_sem\n", this_pid);
                disastrOS_exit(EXIT_FAILURE);
            }
            if (disastrOS_semPost(empty_sem) != 0) {
                printf("CONSUMER [%d] ERROR: disastrOS_semPost empty_sem\n", this_pid);
                disastrOS_exit(EXIT_FAILURE);
            }
       }

    }

    disastrOS_exit(EXIT_SUCCESS);
}

void disastrOS_producer(void* args) {
    shared_buffer* sh_buff = (shared_buffer*) args;
    int this_pid = running->pid;
    int i, j;   // Iterators for the CS

    // Re-open the semaphores in the childs. In semOpen if a semaphore is already opened
    // it will not be reallocated, instead re-used. So here I'm using the same semaphores
    // opend in the parent process.
    int full_sem = disastrOS_semOpen(FULL_SEM_ID, 0);
    if (full_sem < 0) {
        printf("PRODUCER [%d] ERROR: disastrOS_semOpen, full_sem\n", this_pid);
        disastrOS_exit(EXIT_FAILURE);
    }

    int empty_sem = disastrOS_semOpen(EMPTY_SEM_ID, BUFFER_SIZE);
    if (empty_sem < 0) {
        printf("PRODUCER [%d] ERROR: disastrOS_semOpen, empty_sem\n", this_pid);
        disastrOS_exit(EXIT_FAILURE);
    }

    int mutex_sem = disastrOS_semOpen(MUTEX_SEM_ID, 1);
    if (mutex_sem < 0) {
        printf("PRODUCER [%d] ERROR: disastrOS_semOpen, mutex_sem\n", this_pid);
        disastrOS_exit(EXIT_FAILURE);
    }
    
    for (i = 0; i < NUM_ITER; i++) {
        for (j = 0; j < BUFFER_SIZE; j++) {
        
            if (disastrOS_semWait(empty_sem) < 0) {
                printf("PRODUCER [%d] ERROR: disastrOS_semWait empty_sem\n", this_pid);
                disastrOS_exit(EXIT_FAILURE);
            }
            if (disastrOS_semWait(mutex_sem) < 0) {
                printf("PRODUCER [%d] ERROR: disastrOS_semWait mutex_sem\n", this_pid);
                disastrOS_exit(EXIT_FAILURE);
            }

            sh_buff->buff[j]++;
            printf("PRODUCER [%d]:\t Shared buffer at j = %d\t values\t %d\n", this_pid, j, sh_buff->buff[j]);

            //sh_buff->buff[BUFFER_SIZE%i]++;
            //printf("PRODUCER [%d]:\t Shared buffer value at i = %d\t values\t %d\n", this_pid, BUFFER_SIZE%i, sh_buff->buff[BUFFER_SIZE%i]);

            if (disastrOS_semPost(mutex_sem) != 0) {
                printf("PRODUCER [%d] ERROR: disastrOS_semPost mutex_sem\n", this_pid);
                disastrOS_exit(EXIT_FAILURE);
            }
            if (disastrOS_semPost(full_sem) != 0) {
                printf("PRODUECR [%d] ERROR: disastrOS_semPost full_sem\n", this_pid);
                disastrOS_exit(EXIT_FAILURE);
            }
        }
    }

    disastrOS_exit(EXIT_SUCCESS);
}

void disastrOS_semTest(void *args) {
    int test_num = *((int*) args);
    int i;

    // Declare and fill the shared buffer
    shared_buffer sh_buff;
    for (i = 0; i < BUFFER_SIZE; i++) {
        sh_buff.buff[i] = 0;
    }

    int original_buff[BUFFER_SIZE];
    for (i = 0; i < BUFFER_SIZE; i++) {
        original_buff[i] = sh_buff.buff[i];
    }

    // Open the semaphores
    int full_sem = disastrOS_semOpen(FULL_SEM_ID, 0);
    if (full_sem < 0) {
        printf("PARENT ERROR: disastrOS_semOpen, full_sem\n");
        disastrOS_exit(EXIT_FAILURE);
    }

    int empty_sem = disastrOS_semOpen(EMPTY_SEM_ID, BUFFER_SIZE);
    if (empty_sem < 0) {
        printf("PARENT ERROR: disastrOS_semOpen, empty_sem\n");
        disastrOS_exit(EXIT_FAILURE);
    }

    int mutex_sem = disastrOS_semOpen(MUTEX_SEM_ID, 1);
    if (mutex_sem < 0) {
        printf("PARENT ERROR: disastrOS_semOpen, mutex_sem\n");
        disastrOS_exit(EXIT_FAILURE);
    }
    
    if (test_num == 1) {            // 1 Prod - N Cons
        printf("Spawning %d consumers...\n", NUM_CHILDREN);     // Spawn Cons
        for (i = 0; i < NUM_CHILDREN; i++) {
            disastrOS_spawn(disastrOS_consumer, &sh_buff);
        }

        printf("Spawning 1 producer...\n");                     // Spawn Prod
        disastrOS_spawn(disastrOS_producer, &sh_buff);

        // Wait for the children
        for (i = 0; i < NUM_CHILDREN + 1; i++) {
            disastrOS_wait(0, NULL);
        }

        printf("Buffer:\n");
        for (i = 0; i < BUFFER_SIZE; i++) {
            printf("%d ", sh_buff.buff[i]);
        }
        
        printf("\n\n");
        printf("Solution:\n");
        for (i = 0; i < BUFFER_SIZE; i++) {
            original_buff[i] += -((NUM_CHILDREN - 1) * NUM_ITER);
            printf("%d ", original_buff[i]);
        }

        printf("\n\n");

    } else if (test_num == 2) {     // N Prod - 1 Cons
        printf("Spawning 1 consumer...\n");                     // Spawn Cons
        disastrOS_spawn(disastrOS_consumer, &sh_buff);

        printf("Spawning %d producers...\n", NUM_CHILDREN);     // Spawn Prods
        for (i = 0; i < NUM_CHILDREN; i++) {
            disastrOS_spawn(disastrOS_producer, &sh_buff);
        }


        // Wait for the children
        for (i = 0; i < NUM_CHILDREN + 1; i++) {
            disastrOS_wait(0, NULL);
        }

        printf("Buffer:\n");
        for (i = 0; i < BUFFER_SIZE; i++) {
            printf("%d ", sh_buff.buff[i]);
        }
        
        printf("\n\n");
        printf("Solution:\n");
        for (i = 0; i < BUFFER_SIZE; i++) {
            original_buff[i] += (NUM_CHILDREN - 1) * NUM_ITER;
            printf("%d ", original_buff[i]);
        }

        printf("\n\n");

    } else {                    // N Prod - N Cons
        /*
        printf("Spawning %d consumers...\n", NUM_CHILDREN);     // Spawn Cons
        for (i = 0; i < NUM_CHILDREN; i++) {
            disastrOS_spawn(disastrOS_consumer, &sh_buff);
        }
        
        printf("Spawning %d producers...\n", NUM_CHILDREN);     // Spawn Prods
        for (i = 0; i < NUM_CHILDREN; i++) {
            disastrOS_spawn(disastrOS_producer, &sh_buff);
        }
        */

        printf("Spawning %d producers and %d consumers...\n", NUM_CHILDREN, NUM_CHILDREN);     // Spawn Prods
        for (i = 0; i < NUM_CHILDREN; i++) {
            disastrOS_spawn(disastrOS_producer, &sh_buff);  // Pid Odd
            disastrOS_spawn(disastrOS_consumer, &sh_buff);  // Pidd Even
        }

        // Wait for the children
        for (i = 0; i < NUM_CHILDREN * 2; i++) {
            disastrOS_wait(0, NULL);
        }

        printf("Buffer:\n");
        for (i = 0; i < BUFFER_SIZE; i++) {
            printf("%d ", sh_buff.buff[i]);
        }
        
        printf("\n\n");
        printf("Solution:\n");
        for (i = 0; i < BUFFER_SIZE; i++) {
            printf("%d ", original_buff[i]);
        }

        printf("\n\n");
    }

    // Close all opened semaphores
    if (disastrOS_semClose(full_sem) != 0) {
        printf("PARENT ERROR: disastrOS_semClose, full_sem\n");
        disastrOS_exit(EXIT_FAILURE);
    }

    if (disastrOS_semClose(empty_sem) != 0) {
        printf("PARENT ERROR: disastrOS_semClose, empty_sem\n");
        disastrOS_exit(EXIT_FAILURE);
    }

    if (disastrOS_semClose(mutex_sem) != 0) {
        printf("PARENT ERROR: disastrOS_semClose, mutex_sem\n");
        disastrOS_exit(EXIT_FAILURE);
    }

    // Shutdown the system
    printf("PARENT: Well done! Test completed!\n\n");
    disastrOS_shutdown();
}
/*************************************************************************/


int main(int argc, char** argv) {
    int test_num;
    
    char* logfilename=0;
    if (argc>1) {
        logfilename = argv[1];
    }
   
    // tests
    printf("What would you like to test?\n    1 - semOpen\n    2 - semClose\n    3 - semWait\n    4 - semPost\n    5 - Complete test menu\n");

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
    } else if (test_num == 4) {
        printf("Testing semPost\n");
        disastrOS_start(sempost_test, 0, logfilename);
    } else if (test_num == 5) {
        printf("\nHere you can test all the different models of Producer-Consumer\n\n");
        printf("1 - 1 Producer  and N Consumers\n2 - N Producers and 1 Consumer\n3 - N Producers and N Consumers\n\n");

        int compl_test_num = 0;
        scanf("%d", &compl_test_num);
        if (compl_test_num == 1) {
            printf("\nTEST NO.1: 1 Prod - N Cons\n\n");
            disastrOS_start(disastrOS_semTest, &compl_test_num, logfilename);
        } else if (compl_test_num == 2) {
            printf("\nTEST NO.2: N Prod - 1 Cons\n\n");
            disastrOS_start(disastrOS_semTest, &compl_test_num, logfilename);
        } else if (compl_test_num == 3) {
            printf("\nTEST NO.3: N Prod - N Cons\n\n");
            disastrOS_start(disastrOS_semTest, &compl_test_num, logfilename);
        } else {
            printf("\nWrong input. Sayonara!\n\n");
        }

    } else {
        printf("\nWrong input. Sayonara!\n\n");
    }

    return 0;
}