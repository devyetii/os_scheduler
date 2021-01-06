#include "signal.h"
#include "lib/clock.h"
#include "lib/ipc.h"

/* Modify this file as needed*/
int remainingtime;

bool frozen = false;


void handleStateChange(int signum) {
    // pre-emption
    if (!frozen) {
        printf("Leh kda ysta :( %d\n", remainingtime);
        // set frozen flag
        frozen = true;

        // Write remainingTime
        setRemainingTime(remainingtime);

        printf("After set\n");
    }
    // Resuming 
    else {
        printf("Ana geeet :)) %d\n", remainingtime);
        frozen = false;
    }

    signal(SIGUSR1, handleStateChange);
}

int main(int agrc, char * argv[])
{
    initClk();
    
    // Initialize shared memory for remaining time
    initRemainingTimeCommunication(false /*i.e. not the creator*/);

    // Handle SIGUSR1 sent by the scheduler on pre-emption and resuming
    signal(SIGUSR1, handleStateChange);
    
    // Initially get the value  of remaining time
    remainingtime = atoi(argv[1]);
    printf("Process %d started at %d with remaining_time %d\n", getpid(), getClk(), remainingtime);

    // Use clock changes to change this guy
    int old_clock = getClk(), cur_clock = -1;
    
    while (remainingtime > 0)
    {
        // Load current clock
        cur_clock = getClk();

        // Change remaining time only when clock has changed
        if ((cur_clock > old_clock) && !frozen) {
            remainingtime-= (cur_clock - old_clock);    // Just for safety. I know it can be just one
            printf("Ana 7ayyyy %d\n", remainingtime);
        }

        // Update clocks
        old_clock = cur_clock;
    }
    
    destroyClk(false);
    
    return 0;
}
