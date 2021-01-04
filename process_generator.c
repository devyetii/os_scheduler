#include "lib/clock.h"
#include "lib/data_structures.h"
#include "lib/children.h"
#include "lib/io.h"
#include "lib/scheduling_algorithms.h"

void clearResources(int);

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);

    // Process data queue
    FIFOQueue* fq = FIFOQueue__create();
    
    // 1. Read the input files.
    FILE* f = openFile("t.txt", "r");

    while (!isEndOfFile(f)) {
        ProcessData* pd = readProcess(f);
        FIFOQueue__push(fq, pd);
    }

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    int algo, q;
    printf("Please enter the algorithm needed (0: HPF - 1: SRTN - 2: RR): ");
    scanf("%d", &algo);

    if (algo == RR) {
        printf("Please enter the Q for RR algorithm: ");
        scanf("%d", &q);
    }

    // 3. Initiate and create the scheduler and clock processes.
    int clkPid = createChild("./clk.out", 0, 0);
    int schPid = createChild("./scheduler.out", algo, q);
    printf("Clock created with pid %d\n", clkPid);
    printf("Scheduler created with pid %d\n", schPid);

    // 4. Use this function after creating the clock process to initialize clock
    initClk();

    // To get time use this
    int x = getClk();
    printf("current time is %d\n", x);

    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    exit(-1);
}
