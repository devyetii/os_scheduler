#include "lib/clock.h"
#include "lib/ipc.h"
#include "lib/data_structures.h"
#include "lib/process_management.h"
#include "lib/remaining_time.h"


int msg_q_id, sem_id;

bool finished = false;    

FIFOQueue* fq;

PCB* process_in_progress = NULL;

bool process_finished = false;

int cur_clock;

void handler(int signum) {
    finished = true;
}

// Handle process finishing
void handleProcessFinished(int signum) {
    int* status;
    int pid = wait(status);

    printf("Process %d finished at %d\n", process_in_progress->p_data.pid, cur_clock);

    // TODO :: PRINTING AND STATISTICS
    
    PCB__destroy(process_in_progress);

    process_in_progress = NULL;
}

int main(int argc, char * argv[])
{
    initClk();

    // Sent by the process_generator when it finishes all processes
    signal(SIGCHLD, handleProcessFinished);
    signal(SIGUSR1, handler);

    // Init q
    msg_q_id = getProcessMessageQueue(KEYSALT);
    sem_id = getSem(KEYSALT);

    // Init shared memory for remaining time
    initRemainingTimeCommunication(true /* i.e. the creator */);

    fq = FIFOQueue__create();

    // Looop
    int old_clock = getClk();
    while (1)
    {
        // Handeling Message comming from process_generator
        
        if (__down(sem_id) != -1) {
            ProcessData recievedProcess = recieveProcessMessage(msg_q_id, SCHEDULER_TYPE);
            while (recievedProcess.pid != -1) {
                ProcessData__print(&recievedProcess);

                // create the PCB
                PCB* p = PCB__create(recievedProcess, recievedProcess.t_running, 0, IDLE);
                
                // Push it to the q (depending on the algorithm)
                FIFOQueue__push(fq, p);

                recievedProcess = recieveProcessMessage(msg_q_id, SCHEDULER_TYPE);
            }
        }
        
        // Update clock
        cur_clock = getClk();

        // process processes in case of clock updated and no on-going process
        if (cur_clock > old_clock && process_in_progress == NULL && !FIFOQueue__isEmpty(fq)) {
            PCB* p = FIFOQueue__peek(fq);
            if (p->p_data.t_arrival <= cur_clock) {     // (<) just for saftey

                // Update process in progress
                FIFOQueue__pop(fq);
                process_in_progress = p;
                p->state = 0;

                // TODO :: WRITING AND STATS

                // Fire the process
                createChild("./process.out", p->t_remaining, 0);
                printf("Scheduler fired process %d at %d\n", p->p_data.pid, cur_clock);
            }
        }

        // When to exit
        if (finished && process_in_progress == NULL && FIFOQueue__isEmpty(fq)) break;
        // Save latest clock
        old_clock = cur_clock;
    }

    destroyRemainingTimeCommunication(true /* i.e. the creator */);
    destroyClk(false);
}
