# disastrOS_semaphores
Project for the Operative Systems course in Computer Engineering, Sapienza - University of Rome. Implementation of semaphores for disastrOS.

##  Syscalls implemented
I implemented some system calls to manage the semaphores in the operative system DisastrOS, developed by teacher Giorgio Grisetti and his team. These syscalls are:

- int disastrOS_semOpen(int semnum, int count): 
    Creates a semaphore in the system, having num semnum
    the semaphore is accessible throughuot the entire system
    by its id.
    on success, the function call returns semnum (>=0);
    in failure the function returns an error code <0.

- int disastrOS_semClose(int semnum):
    Releases from an application the given semaphore (ID = semnum),
    returns 0 on success,
    returns an error code if the semaphore is not owned by the application.

- int disastrOS_semWait(int semnum):
    Decrements the given semaphore
    if the semaphore is 0, the caller is put onto wait
    returns an error code

- int disastrOS_semPost(int semnum):
    Increments the given semaphore
    if the semaphore was at 0, and some other thread was waiting
    the thread is resumed
    returns 0 on success, an error code on failure 

## How to Run
To compile simply use make while to test digit:
    
    ./disastrOS_my_semtest

Then:\
     1) Test semOpen\
     2) Test semClose\
     3) Test semWait\
     4) Test semPost\
     5) Complete test menu\
     6) Test multi semClose

In the complete test menu, you can test a producer - consumer model.
The number of producers and consumers can be chosen arbitrarily simply by typing it. (They can be different)
