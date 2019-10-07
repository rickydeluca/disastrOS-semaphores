#include    <stdio.h>
#include    <stdlib.h>
#include    <unistd.h>

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
#define     MUTEX_CONS_ID       2
#define     MUTEX_PROD_ID       3
#define     TEST_SEM_ID         4

#define     BUFFER_SIZE         10
#define     NUM_OPERATIONS      10


// To avoid to have the ready list empty
void sleeperFunction(   void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

// Function for avoiding compulsive stamps
void dreamALittleDream(int times) {
    int i = 0;
    for (i = 0; i < times; i++) {
        sleep(1);
    }

    printf("\n");
}

typedef struct shared_data {
    int buff[BUFFER_SIZE];
    int num_ops_cons;
    int num_ops_prod;
} shared_data;

// Shared var to track the process
int cons_idx, prod_idx;
int num_prod, num_cons;

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
    int mutex_sem = disastrOS_semOpen(mutex_sem_id, 1);

    /* Debug print to check the fd value
    printf("CONSUMER:\n");
    printf("\t - mutex_sem_fd: %d\n", mutex_sem);
    */

    int i;
    for (i = 0; i < num_iter; i++) {
        // Check on entry
        //printf("CONSUMER: Just before wait\n");
        disastrOS_semWait(mutex_sem);

        // Access from consumer to the shared variable
        (*sh_var)++;
        printf("CONSUMER:\t Shared variable value:\t %d\n\n", shared_var);

        //printf("CONSUMER: Just before post\n");
        disastrOS_semPost(mutex_sem);
    }

    disastrOS_exit(EXIT_SUCCESS);
}

void sempostTest_producer(void* args) {  
    int* sh_var = (int*) args;
    int mutex_sem = disastrOS_semOpen(mutex_sem_id, 1);

    /* Debug print to check the fd value
    printf("PRODUCER:\n");
    printf("\t - mutex_sem_fd: %d\n", mutex_sem);
    */

    int i;
    for (i = 0; i < num_iter; i++) {
        // Check on entry
        //printf("PRODUCER: Just before wait\n");
        disastrOS_semWait(mutex_sem);

        // Access from consumer to the shared buffer
        (*sh_var)--;
        printf("PRODUCER:\t Shared variable value:\t %d\n\n", shared_var);

        //printf("PRODUCER: Just before post\n");
        disastrOS_semPost(mutex_sem);
    }

    disastrOS_exit(EXIT_SUCCESS);
}   

void sempost_test(void* args) {
    printf ("Testing the semPost using a simple Producer Consumer model that read and write in a buffer\n");

    // Open the semaphore in the main process so I can close them later
    int mutex_sem_fd = disastrOS_semOpen(mutex_sem_id, 1);

    /* Debug print to check the fd value
    printf("PARENT:\n");
    printf("\t - mutex_sem_fd: %d\n\n", mutex_sem_fd);
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
    disastrOS_semClose(mutex_sem_fd);

    printf("TEST SUCCESSFUL: semPost\n");
    disastrOS_shutdown();
}
/*************************************************************************/

/************************** COMPLETE TEST ********************************/
void disastrOS_consumer(void* args) {
    shared_data* sd = (shared_data*) args;
    //int test_num = sd->test_num;
    int this_pid = running->pid;
    int i;    // Index for consumer operation

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

    int mutex_cons;
    if (num_cons > 1) {   // NUM CONSUMERS > 1
        mutex_cons = disastrOS_semOpen(MUTEX_CONS_ID, 1);
        if (mutex_cons < 0) {
            printf("CONSUMER [%d] ERROR: disastrOS_semOpen, mutex_cons\n", this_pid);
            disastrOS_exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < sd->num_ops_cons; i++) {

        if (disastrOS_semWait(full_sem) < 0) {
            printf("CONSUMER [%d] ERROR: disastrOS_semWait, full_sem\n", this_pid);
            disastrOS_exit(EXIT_FAILURE);
        }

        // Wait on Mutex only if NUM CONS > 1
        if (num_cons > 1) {
            if (disastrOS_semWait(mutex_cons) < 0) {
                printf("CONSUMER [%d] ERROR: disastrOS_semWait, mutex_cons\n", this_pid);
                disastrOS_exit(EXIT_FAILURE);
            }
        }
        
        printf("CONSUMER [%d] in CS\n", this_pid);
        
        sd->buff[cons_idx]--;
        printf("CONSUMER [%d]: Buffer[%d] =\t %d\n", this_pid, cons_idx%BUFFER_SIZE, sd->buff[cons_idx%BUFFER_SIZE]);
        cons_idx = (cons_idx + 1) % BUFFER_SIZE;

        // Same on post
        if (num_cons > 1) {
            if (disastrOS_semPost(mutex_cons) != 0) {
                printf("CONSUMER [%d] ERROR: disastrOS_semPost mutex_cons\n", this_pid);
                disastrOS_exit(EXIT_FAILURE);
            }
        }

        if (disastrOS_semPost(empty_sem) != 0) {
            printf("CONSUMER [%d] ERROR: disastrOS_semPost empty_sem\n", this_pid);
            disastrOS_exit(EXIT_FAILURE);
        }

        printf("CONSUMER [%d] out from CS\n\n", this_pid);
    }

    disastrOS_exit(disastrOS_getpid()+1);
}

void disastrOS_producer(void* args) {
    shared_data* sd = (shared_data*) args;
    int this_pid = running->pid;
    int i;

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
    
    int mutex_prod;
    if (num_prod > 1) {       // NUM PRODUCERS > 1
        mutex_prod = disastrOS_semOpen(MUTEX_PROD_ID, 1);
        if (mutex_prod < 0) {
            printf("PRODUCER [%d] ERROR: disastrOS_semOpen, mutex_prod\n", this_pid);
            disastrOS_exit(EXIT_FAILURE);
        }
    }
    
    for (i = 0; i < sd->num_ops_prod; i++) {

        if (disastrOS_semWait(empty_sem) < 0) {
            printf("PRODUCER [%d] ERROR: disastrOS_semWait empty_sem\n", this_pid);
            disastrOS_exit(EXIT_FAILURE);
        }

        // Wait on Mutex only if NUM PRODS > 1
        if (num_prod > 1) {
            if (disastrOS_semWait(mutex_prod) < 0) {
                printf("PRODUCER [%d] ERROR: disastrOS_semWait mutex_prod\n", this_pid);
                disastrOS_exit(EXIT_FAILURE);
            }
        }

        printf("PRODUCER [%d] in CS\n", this_pid);
        
        sd->buff[prod_idx]++;
        printf("PRODUCER [%d]: Buffer[%d] =\t %d\n", this_pid, prod_idx%BUFFER_SIZE, sd->buff[prod_idx%BUFFER_SIZE]);
        prod_idx = (prod_idx + 1) % BUFFER_SIZE;
        
        // Same on post
        if (num_prod > 1) {
            if (disastrOS_semPost(mutex_prod) != 0) {
                printf("PRODUCER [%d] ERROR: disastrOS_semPost mutex_prod\n", this_pid);
                disastrOS_exit(EXIT_FAILURE);
            }
        }

        if (disastrOS_semPost(full_sem) != 0) {
            printf("PRODUECR [%d] ERROR: disastrOS_semPost full_sem\n", this_pid);
            disastrOS_exit(EXIT_FAILURE);
        }

        printf("PRODUCER [%d] out from CS\n\n", this_pid);
    }

    disastrOS_exit(disastrOS_getpid()+1);
}

void disastrOS_semTest(void *args) {
    int i;

    prod_idx = 0;
    cons_idx = 0;

    // Declare and fill the shared struct to pass
    shared_data data_to_pass;
    for (i = 0; i < BUFFER_SIZE; i++) {
        data_to_pass.buff[i]    = 0;
    }
    // Number of effective ops for the producers and the consumers must be the same
    data_to_pass.num_ops_cons = num_prod;   //* NUM_OPERATIONS;
    data_to_pass.num_ops_prod = num_cons;   //* NUM_OPERATIONS;
   
    // Create a copy of the original buffer to confront the solutions
    int solution_buff[BUFFER_SIZE];
    for (i = 0; i < BUFFER_SIZE; i++) {
        solution_buff[i] = 0;
    }
    
    // Start the sleeper process
    disastrOS_spawn(sleeperFunction, 0);

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

    int mutex_cons = disastrOS_semOpen(MUTEX_CONS_ID, 1);
    if (mutex_cons < 0) {
        printf("PARENT ERROR: disastrOS_semOpen, mutex_cons\n");
        disastrOS_exit(EXIT_FAILURE);
    }

    int mutex_prod = disastrOS_semOpen(MUTEX_PROD_ID, 1);
    if (mutex_prod < 0) {
        printf("PARENT ERROR: disastrOS_semOpen, mutex_prod\n");
        disastrOS_exit(EXIT_FAILURE);
    }
    
    
    // N Prod - M Cons    
    printf("Spawning %d consumers...\n", num_cons);     // Spawn Cons
    for (i = 0; i < num_cons; i++) {
        disastrOS_spawn(disastrOS_consumer, &data_to_pass);
    }
    
    printf("Spawning %d producers...\n", num_prod);     // Spawn Prods
    for (i = 0; i < num_prod; i++) {
        disastrOS_spawn(disastrOS_producer, &data_to_pass);
    }

    // Wait for the children
    for (i = 0; i < num_cons + num_prod; i++) {
        disastrOS_wait(0, NULL);
    }

    // Print solution 
    printf("Buffer:\n");
    for (i = 0; i < BUFFER_SIZE; i++) {
        printf("%d ", data_to_pass.buff[i]);
    }
    
    printf("\n\n");
    printf("Solution:\n");
    for (i = 0; i < BUFFER_SIZE; i++) {
        printf("%d ", solution_buff[i]);
    }

    printf("\n\n");
    

    // Close all opened semaphores
    if (disastrOS_semClose(full_sem) != 0) {
        printf("PARENT ERROR: disastrOS_semClose, full_sem\n");
        disastrOS_exit(EXIT_FAILURE);
    }

    if (disastrOS_semClose(empty_sem) != 0) {
        printf("PARENT ERROR: disastrOS_semClose, empty_sem\n");
        disastrOS_exit(EXIT_FAILURE);
    }

    if (disastrOS_semClose(mutex_cons) != 0) {
        printf("PARENT ERROR: disastrOS_semClose, mutex_cons\n");
        disastrOS_exit(EXIT_FAILURE);
    }

    if (disastrOS_semClose(mutex_prod) != 0) {
        printf("PARENT ERROR: disastrOS_semClose, mutex_prod\n");
        disastrOS_exit(EXIT_FAILURE);
    }

    // Shutdown the system
    printf("PARENT: Well done! Test completed!\n\n");
    disastrOS_shutdown();
}
/*************************************************************************/

/************************** MULTI SEM CLOSE ******************************/
void closeTheSem(void* args) {

    int sem_test = disastrOS_semOpen(TEST_SEM_ID, 1);
     if (sem_test < 0) {
        printf("CHILD ERROR: disastrOS_semOpen, sem_test\n");
        disastrOS_exit(DSOS_ERESOURCEOPEN);
    }

    if (disastrOS_semClose(sem_test) != 0) {
        printf("CHILD ERROR: disastrOS_semClose, sem_test\n");
        disastrOS_exit(DSOS_ERESOURCECLOSE);
    }

    disastrOS_exit(disastrOS_getpid());
}

void multiSemCloseTest(void* args) {
    int num_proc = *((int*) args);
    int i = 0;

    printf("Status before opening the semaphore\n");
    disastrOS_printStatus();

    printf("Opening the semaphore\n");
    
    // Open the semaphores
    int sem_test = disastrOS_semOpen(TEST_SEM_ID, 1);
    if (sem_test < 0) {
        printf("PARENT ERROR: disastrOS_semOpen, sem_test\n");
        disastrOS_exit(DSOS_ERESOURCEOPEN);
    }

    dreamALittleDream(20);
    
    printf("Status after opening the semaphore\n");
    disastrOS_printStatus();
    
    printf("Closing multiple times the same semaphores with %d processes\n", num_proc);
    // Spawn the processes
    for (i = 0; i < num_proc; i++) {
        disastrOS_spawn(closeTheSem, 0);
    }

    dreamALittleDream(20);

    // Wait for the processes
    for (i = 0; i < num_proc; i++) {
        disastrOS_wait(0, NULL);
    }

    printf("Status:\n");
    disastrOS_printStatus();

    printf("Closing the semaphore in the main\n");
    if (disastrOS_semClose(sem_test) != 0) {
        printf("PARENT ERROR: disastrOS_semClose, sem_test\n");
        disastrOS_exit(DSOS_ERESOURCECLOSE);
    }

    dreamALittleDream(20);

    printf("Status:\n");
    disastrOS_printStatus();

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
    printf("What would you like to test?\n    1 - semOpen\n    2 - semClose\n    3 - semWait\n    4 - semPost\n    5 - Complete test menu\n    6 - Multi semClose\n");

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

        printf("Insert number of producers: ");
        scanf("%d", &num_prod);
        printf("\n");
        
        printf("Insert nuber of consumers: ");
        scanf("%d", &num_cons);
        printf("\n\n");

        disastrOS_start(disastrOS_semTest, 0, logfilename);
    } else if (test_num == 6) {
        printf("Tring to close the same semaphore with different processes\n\n");

        int num_proc = 0;
        printf("Insert how many processes should close the semaphore: ");
        scanf("%d", &num_proc);
        printf("\n\n");

        disastrOS_start(multiSemCloseTest, &num_proc, logfilename);

    } else {
        printf("\nWrong input. Sayonara!\n\n");
    }

    return 0;
}