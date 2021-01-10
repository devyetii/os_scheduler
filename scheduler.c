#include "lib/clock.h"
#include "lib/ipc.h"
#include "lib/data_structures.h"
#include "lib/process_management.h"
#include "lib/remaining_time.h"
///////////////////////////// global variables ////////////////
int msg_q_id, finished = 0, t_rem, sem_id; // type should change with argv
bool flag_in_run = false, p_need_fill = false, set_Data = false, comp = true, chang_in_child = false;
/////////////////// functions handle signals /////////////////
void handler(int signum)
{
    finished = 1;
    signal(SIGUSR1, handler);
}

void handleProcessFinished(int signum)
{
    if (chang_in_child)
        chang_in_child = false;
    else
        p_need_fill = true; // then in while change all values
    signal(SIGCHLD, handleProcessFinished);
}
void stopProcess(int sinum)
{
    if (comp)
        t_rem = getReminingTime();
    else
        set_Data = true; //set data & stop process again
    signal(SIGUSR2, stopProcess);
}
/////////////////////////using function/////////////
void checkComeShortest(PriorityQueue *PQ, int current_child_pid)
{
    PCB *process = PriorityQueue__peek(PQ);
    if (process->t_remaining < getReminingTime())
    {
        comp = false;
        kill(current_child_pid, SIGUSR1);
    }
}

void insert_all_comming_in_PQueue(PriorityQueue *PQ, int type)
{
    int pid = 0;
    while (pid != -1)
    {
        ProcessData recievedProcess = recieveProcessMessage(msg_q_id, SCHEDULER_TYPE);
        pid = recievedProcess.pid;
        if (pid != -1)
        {
            PCB *process = PCB__create(recievedProcess, recievedProcess.t_running, -1, -1, IDLE, -1, -1);
            if (type == 0)
            {
                long long p = recievedProcess.priority;
                p <<= 32;
                p |= (-1 * recievedProcess.t_arrival) & 0xFFFFFFFF;
                PriorityQueue__push(PQ, process, p);
            }
            else if (type == 1) ///sRTN
                PriorityQueue__push(PQ, process, -1 * recievedProcess.t_running);
        }
    }
}
////////////////////////////////////////////

int main(int argc, char *argv[])
{
    PCB *current_process; // current process in run
    PriorityQueue *PQ = PriorityQueue__create(10000);
    initRemainingTimeCommunication(true);
    int type = atoi(argv[1]);
    initClk();
    signal(SIGCHLD, handleProcessFinished);
    signal(SIGUSR1, handler);
    signal(SIGUSR2, stopProcess);
    msg_q_id = getProcessMessageQueue(KEYSALT);
    //sem_id = getSem(KEYSALT);
    printf("Recieved queue with id %d\n", msg_q_id);
    while (1)
    {
        if (p_need_fill)
        {

            flag_in_run = false;
            p_need_fill = false;
            printf("Process %d finished at %d\n", current_process->p_data.pid, getClk());
            // TODO :: PRINTING AND STATISTICS
            PCB__destroy(current_process);
            current_process = NULL;
            // set all need values
        }
        else if (set_Data)
        {
            // another status
            current_process->state = STOPPED;
            current_process->t_remaining = t_rem;
            printf("Process %d switch at %d\n", current_process->p_data.pid, getClk());
            flag_in_run = false;
            set_Data = false;
            chang_in_child = true;
            kill(current_process->actual_pid, SIGSTOP);
            PriorityQueue__push(PQ, current_process, -1 * current_process->t_remaining);
        }
        if (!flag_in_run && !PriorityQueue__isEmpty(PQ))
        {
            insert_all_comming_in_PQueue(PQ, type);
            current_process = PriorityQueue__pop(PQ);
            flag_in_run = true;
            comp = true;
            t_rem = current_process->t_remaining;
            if (current_process->actual_pid == -1)
            {
                current_process->t_st = getClk();                                                                                      // set all others
                current_process->actual_pid = createChild("./process.out", current_process->t_remaining, current_process->p_data.pid); // anything else process nee
            }
            else
            {
                chang_in_child = true;
                kill(current_process->actual_pid, SIGCONT);
                kill(current_process->actual_pid, SIGUSR1);
            }
        }
        if (finished != 2 && (type != 0 || !flag_in_run))
        {
            if (finished == 1)
                finished == 2;
            insert_all_comming_in_PQueue(PQ, type);
        }
        if (type == 1 && !PriorityQueue__isEmpty(PQ) && flag_in_run)
            checkComeShortest(PQ, current_process->actual_pid);
    }
    destroyClk(false);
}
