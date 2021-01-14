#include "signal.h"
#include "lib/clock.h"
#include "lib/ipc.h"
#include "lib/remaining_time.h"

/* Modify this file as needed*/
int remainingtime, old_clock, cur_clock;
bool stop = false;
void clearIPC(int signum)
{
    destroyClk(false);
    exit(0);
    signal(SIGINT, clearIPC);
}

void setClock(int signum)
{
    old_clock = getClk();
    cur_clock = -1;
}

int main(int agrc, char *argv[])
{
    initClk();
    // Initialize shared memory for remaining time
    // Handle SIGUSR1 sent by the scheduler on pre-emption
    signal(SIGCONT, setClock);
    signal(SIGINT, clearIPC);
    // Initially get the value  of remaining time
    remainingtime = atoi(argv[1]);
    printf("processID %d real %d  start at %d remanningTime %d \n", atoi(argv[2]), getpid(), getClk(), atoi(argv[1]));
    // Use clock changes to change the remaining time
    old_clock = getClk();
    cur_clock = -1;

    while (remainingtime > 0)
    {
        // Load current clock
        cur_clock = getClk();
        // Change remaining time only when clock has changed
        if ((cur_clock > old_clock))
        {
            remainingtime -= 1;
        }
        // Update clocks
        old_clock = cur_clock;
    }
    destroyClk(false);
    return 0;
}
