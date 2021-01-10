#include "signal.h"
#include "lib/clock.h"
#include "lib/ipc.h"
#include "lib/remaining_time.h"

/* Modify this file as needed*/
int remainingtime;

void clearIPC(int signum) {
    destroyRemainingTimeCommunication(false);
    destroyClk(false);
    exit(0);
}

void handleStateChange(int signum) {
    // Write remainingTime
    setRemainingTime(remainingtime);

    // Notify the parent that writing have finished
    kill(getppid(), SIGUSR2);
    signal(SIGUSR1, handleStateChange);
}

int main(int agrc, char * argv[])
{
    initClk();
    
    // Initialize shared memory for remaining time
    initRemainingTimeCommunication(false /*i.e. not the creator*/);

    // Handle SIGUSR1 sent by the scheduler on pre-emption
    signal(SIGUSR1, handleStateChange);
    signal(SIGINT, clearIPC);

    // Initially get the value  of remaining time
    remainingtime = atoi(argv[1]);

    // Use clock changes to change the remaining time
    int old_clock = getClk(), cur_clock = -1;
    
    while (remainingtime > 0)
    {
        // Load current clock
        cur_clock = getClk();

        // Change remaining time only when clock has changed
        if ((cur_clock > old_clock)) {
            remainingtime-= 1;
        }

        // Update clocks
        old_clock = cur_clock;
    }
    
    destroyRemainingTimeCommunication(false);
    destroyClk(false);
    
    return 0;
}
