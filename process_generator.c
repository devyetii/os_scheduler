#include "lib/clock.h"
#include "lib/data_structures.h"
#include "lib/process_management.h"
#include "lib/io.h"
#include "lib/scheduling_algorithms.h"
#include "lib/ipc.h"
#include "lib/safeExit.h"

void clearResources(int signum);

// File ptr
FILE* f;

// Q+sem for communication with the scheduler
int msg_q_id, sem_id, schPid;

void initiateClock(int signum)
{
    int clkPid = createChild("./clk.out", 0, 0);
    initClk();
    kill(schPid, SIGUSR1);
}

int main(int argc, char *argv[])
{
    // Handle both SIGINT and SIGSEGV to avoid leakage of resources in case od segmentation faults
    signal(SIGINT, clearResources);
    signal(SIGSEGV, clearResources);
    signal(SIGUSR1, initiateClock);
    // signal(SIGCHLD, finish);

    // get the message queue and sem
    msg_q_id = getProcessMessageQueue(KEYSALT);
    sem_id = getSem(KEYSALT);

    // create ProcessData queue
    FIFOQueue *fq = FIFOQueue__create();

    // Read the input files.
    f = openFile("processes.txt", "r");

    while (!isEndOfFile(f))
    {
        ProcessData *pd = readProcess(f);
        if (pd != NULL) {
            ProcessData__print(pd);
            FIFOQueue__push(fq, pd);
        }
    }

    // Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    int algo = 0, q = 0;
    printf("Please enter the algorithm needed (0: HPF - 1: SRTN - 2: RR): ");
    scanf("%d", &algo);

    if (algo == RR)
    {
        printf("Please enter the Q for RR algorithm: ");
        scanf("%d", &q);
    }

    schPid = createChild("./scheduler.out", algo, q);

    // Main loop, untill all the processes get sent to the scheduler
    int newClock, oldClock = -1;
    while (!FIFOQueue__isEmpty(fq))
    {
        // Update clock
        newClock = getClk();

        if (newClock > oldClock)
        {
            // Only process in case of clock being updated
            ProcessData *top_pd = FIFOQueue__peek(fq);
            while (!FIFOQueue__isEmpty(fq) && top_pd->t_arrival <= newClock)
            {
                // In case the process time has come, pop from the queue and send to the scheduler
                FIFOQueue__pop(fq);
                sendProcessMessage(createProcessMessage(SCHEDULER_TYPE, *top_pd), msg_q_id);
                // ProcessData__destroy(top_pd);
                top_pd = FIFOQueue__peek(fq);
            }
            __up(sem_id);
        }
        oldClock = newClock;
    }

    // signal the schedular that all the processes are sent
    kill(schPid, SIGUSR1);

    // Wait for the Scheduler to finish
    waitForChild(schPid);

    printf("All Done Successfully. Exiting ...\n");

    // Kill the system :skull_and_crossbones:
    destroyClk(true);
    closeFile(f);
}

void clearResources(int signum)
{
    deleteProcessMessageQueue(msg_q_id);
    deleteSemSet(sem_id);
    destroyClk(true);
    closeFile(f);
    safeExit(-1);
}