#include "signal.h"
#include "lib/clock.h"
#include "lib/ipc.h"
#include "lib/remaining_time.h"

/* Modify this file as needed*/
int remainingtime, old_clock, cur_clock;

bool frozen = false;

void handleStateChange(int signum)
{
    // pre-emption
    if (frozen == 0)
    {
        printf("Process %d paused. remaining_time =  %d\n", getpid(), remainingtime);
        // set frozen flag
        frozen = 1;

        // Write remainingTime
        setRemainingTime(remainingtime);

        // Notify the parent that writing have finished
        kill(getppid(), SIGUSR2);
    }
    // Resuming
    else
    {
        printf("Process %d continued. remaining_time = %d\n", getpid(), remainingtime);
        frozen = false;
        old_clock = cur_clock = getClk();
    }

    signal(SIGUSR1, handleStateChange);
}

int main(int agrc, char *argv[])
{
    initClk();

    // Initialize shared memory for remaining time
    initRemainingTimeCommunication(false /*i.e. not the creator*/);

    // Handle SIGUSR1 sent by the scheduler on pre-emption and resuming
    signal(SIGUSR1, handleStateChange);

    // Initially get the value  of remaining time
    remainingtime = atoi(argv[1]);
    printf("Process %d started. remaining_time =  %d\n", getpid(), remainingtime);

    // Use clock changes to change this guy
    old_clock = getClk(), cur_clock = -1;

    while (remainingtime > 0)
    {
        // Load current clock
        cur_clock = getClk();

        // Change remaining time only when clock has changed
        if ((cur_clock > old_clock) && !frozen)
        {
            remainingtime -= (cur_clock - old_clock); // Just for safety. I know it can be just one
            if (remainingtime < 0)
                remainingtime = 0;
            // Write remainingTime
            setRemainingTime(remainingtime);
            kill(getppid(), SIGUSR2);
        }

        // Update clocks
        old_clock = cur_clock;
    }
    printf("terminated in %d\n ", getClk());
    destroyClk(false);

    return 0;
}
