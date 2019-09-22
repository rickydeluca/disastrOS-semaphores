#include <stdio.h>
#include <stdlib.h>

#include "disastrOS.h"
#include "disastrOS_semaphore.h"

#define EXIT_FAILURE        1
#define EXIT_SUCCESSFUL     0

/* SEMOPEN TEST */
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

/* SEMCLOSE TEST */
void semclose_test(void* args) {
    
    int sem_id  = 1;
    int count   = 1;

    int sem_fd  = 0;

    printf("Opening semaphore with ID: %d...\n", sem_id);

    sem_fd = disastrOS_semOpen(sem_id, count);
    if (sem_fd < 0) {
        printf("TEST ERROR: semOpen\n");
        exit(EXIT_FAILURE);
    }

    //printf(" Done!\n");

    printf ("Closing the semaphore with ID: %d...\n", sem_id);

    if (disastrOS_semClose(sem_fd) != 0) {
        printf("TEST ERROR: semClose\n");
        exit(EXIT_FAILURE);
    }

    //printf(" Done!\n");

    printf("TEST SUCCESSFUL: semClose\n");
    exit(EXIT_SUCCESSFUL);
}

int main(int argc, char** argv) {
    int test_num;
    
    char* logfilename=0;
    if (argc>1) {
        logfilename = argv[1];
    }
   
    // TEST: semOpen
    printf("What would you like to test?\n    1 - semOpen\n    2 - semClose\n");

    scanf("%d", &test_num);
    if (test_num == 1) {
        printf("Testing semOpen\n");
        disastrOS_start(semopen_test, 0, logfilename);
    } else if (test_num == 2) {
        printf("Testing semClose\n");
        disastrOS_start(semclose_test, 0, logfilename);
    } else {
        printf("Wrong input. Sayonara!");
    }

    return 0;
}