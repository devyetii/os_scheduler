#include "signal.h"
#include "lib/clock.h"
#include "lib/ipc.h"
#include "lib/remaining_time.h"

/* Modify this file as needed*/
int remainingtime, old_clock;
bool stop = false;

void handleStateChange(int signum)
{
    // Write remainingTime
    setRemainingTime(stop ? remainingtime++ : remainingtime);
    stop = !stop;
    // Notify the parent that writing have finished
    if (stop)
        kill(getppid(), SIGUSR2);
    signal(SIGUSR1, handleStateChange);
}

int main(int agrc, char *argv[])
{
    initClk();

    // Initialize shared memory for remaining time
    initRemainingTimeCommunication(false /*i.e. not the creator*/);
    // Handle SIGUSR1 sent by the scheduler on pre-emption
    signal(SIGUSR1, handleStateChange);

    // Initially get the value  of remaining time
    remainingtime = atoi(argv[1]);
    printf("processID %d start at %d remanningTime %d \n", atoi(argv[2]), getClk(), atoi(argv[1]));
    // Use clock changes to change the remaining time
    old_clock = getClk();
    int cur_clock = -1;

    while (remainingtime > 0)
    {
        // Load current clock
        cur_clock = getClk();

        // Change remaining time only when clock has changed
        if ((cur_clock > old_clock))
        {
            remainingtime -= 1;
            setRemainingTime(remainingtime);
            // Notify the parent that writing have finished
            kill(getppid(), SIGUSR2);
        }

        // Update clocks
        old_clock = cur_clock;
    }
    //printf("trmenate process %d in %d \n", atoi(argv[2]), getClk());
    destroyClk(false);

    return 0;
}
