#include <stdio.h>
#include <stdlib.h>

#include "disastrOS.h"
#include "disastrOS_semaphore.h"

/* Testing semOpen */
void semopen_test() {
    int sem_id  = 0;
    int count   = 0;
    if (disastrOS_semOpen(sem_id, 0) < 0) {
        printf("TEST ERROR: semOpen");
        exit(1);
    }

    printf("TEST SUCCESSFUL: semOpen")
}

int main (int argc, char** argv) {
    
    char* logfilename=0;
    if (argc>1) {
        logfilename = argv[1];
    }
   
    // TEST: semoOpen
    printf("Testing semOpen")
    disastrOS_start(semopen_test, 0, logfilename);

    return 0;
}