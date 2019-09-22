#include <stdio.h>
#include <stdlib.h>

#include "disastrOS.h"
#include "disastrOS_semaphore.h"

/* Testing semOpen */
void semopen_test(void* args) {
    int sem_id  = 1;
    int count   = 1;
    if (disastrOS_semOpen(sem_id, count) < 0) {
        printf("TEST ERROR: semOpen\n");
        exit(1);
    }

    printf("TEST SUCCESSFUL: semOpen\n");
}

int main(int argc, char** argv) {
    
    char* logfilename=0;
    if (argc>1) {
        logfilename = argv[1];
    }
   
    // TEST: semoOpen
    printf("Testing semOpen\n");
    disastrOS_start(semopen_test, 0, logfilename);

    return 0;
}