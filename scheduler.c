#include "lib/clock.h"
#include "lib/ipc.h"
#include "lib/data_structures.h"
#include "lib/process_management.h"
#include "lib/remaining_time.h"
///////////////////////////// global variables ////////////////
PCB *current_process;
int msg_q_id, finished = -1, sem_id, run_clock = -1, old_clock = -1;
bool flag_in_run = false, chang_in_child = false, read_done = false;
/////////////////// functions handle signals /////////////////
void handlerAndIntial();
void handlerFromGenerator(int signum);
void handleProcessFinished(int signum);
void clearIPC(int signum);
/////////////////////////function for all algorithim/////////////
void setFinishedProcesss();
bool processNotFinished(void *q, int type);
void insert_all_comming_in_Queue(void *Q, int type);
void setDateNotFinishedP(void *q, int type);
void startProcess(void *PQ, int type);
///////////////////////////////functions for SRTN /////////////////////
void checkComeShortest(void *PQ, int type);
////////////////////////////////////////////

int main(int argc, char *argv[])
{
    printf("process id for sc %d \n", getpid());

    // current process in run
    void *Queue;
    int type = atoi(argv[1]), quantum;
    if (type != 2)
        Queue = PriorityQueue__create(10000);
    else
    {
        Queue = FIFOQueue__create();
        quantum = atoi(argv[2]);
    }
    handlerAndIntial();
    while (1)
    {
        if (finished > -1) // clk  started
        {
            insert_all_comming_in_Queue(Queue, type);
            if (!flag_in_run && processNotFinished(Queue, type))
                startProcess(Queue, type);

            if (flag_in_run && old_clock < getClk() && type > 0 && read_done)
            {
                printf("enter here old: %d new: %d rem : %d  \n", old_clock, getClk(), current_process->t_remaining);
                current_process->t_remaining -= (getClk() - old_clock);
                old_clock = getClk();
                read_done = false;
                if (current_process->t_remaining != 0)
                    if (type == 1 && processNotFinished(Queue, type))
                        checkComeShortest(Queue, type);
                    else if (type == 2 && getClk() - run_clock == quantum)
                        setDateNotFinishedP(Queue, type);
            }
        }
    }
    destroyClk(false);
}

////////////////////////////////function to handle signal /////////////////////
void handlerAndIntial()
{
    signal(SIGCHLD, handleProcessFinished);
    signal(SIGUSR1, handlerFromGenerator);
    signal(SIGINT, clearIPC);
    msg_q_id = getProcessMessageQueue(KEYSALT);
    sem_id = getSem(KEYSALT);
    printf("Recieved queue with id %d\n", msg_q_id);
    kill(getppid(), SIGUSR1);
}

void handlerFromGenerator(int signum)
{
    finished++;
    if (finished == 0)
        initClk();
    signal(SIGUSR1, handlerFromGenerator);
}

void handleProcessFinished(int signum)
{
    if (chang_in_child)
        chang_in_child = false;
    else
        setFinishedProcesss();
    signal(SIGCHLD, handleProcessFinished);
}
void clearIPC(int signum)
{
    destroyClk(false);
    exit(0);
}
//////////////////// for SRTN ///////////////////////
void checkComeShortest(void *PQ, int type)
{
    PCB *process = PriorityQueue__peek(PQ);
    if (process->t_remaining < current_process->t_remaining)
        setDateNotFinishedP(PQ, type);
}
////////////////////////for HPF SRTN ////////////////////////////////
void startProcess(void *PQ, int type)
{
    if (type != 2)
        current_process = PriorityQueue__pop(PQ);
    else
        current_process = FIFOQueue__pop(PQ);
    flag_in_run = true;
    old_clock = getClk();
    run_clock = getClk();
    read_done = false;
    if (current_process->actual_pid == -1)
    {
        current_process->t_st = getClk();
        current_process->actual_pid = createChild("./process.out", current_process->t_remaining, current_process->p_data.pid);
    }
    else
    {
        chang_in_child = true;
        kill(current_process->actual_pid, SIGCONT);
        //out file & update data
    }
}

void insert_all_comming_in_Queue(void *PQ, int type)
{
    if (__down(sem_id) != -1)
    {
        read_done = true;
        if (finished != 2)
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
                    else if (type == 2)
                        FIFOQueue__push(PQ, process);
                }
            }
            if (finished == 1)
                finished = 2;
        }
    }
}
///////////////////////////////function for all algorithim////////////////////////////////
void setFinishedProcesss()
{
    flag_in_run = false;
    current_process->t_remaining = 0;
    printf("Process %d real %d finished at %d\n", current_process->p_data.pid, current_process->actual_pid, getClk());
    PCB__destroy(current_process);
    current_process = NULL;
}

bool processNotFinished(void *q, int type)
{
    if (type != 2)
        return !PriorityQueue__isEmpty(q);
    return !FIFOQueue__isEmpty(q);
}

void setDateNotFinishedP(void *q, int type)
{
    current_process->state = STOPPED;
    printf("Process %d real %d switch at %d\n", current_process->p_data.pid, current_process->actual_pid, getClk());
    chang_in_child = true;
    kill(current_process->actual_pid, SIGSTOP);
    flag_in_run = false;
    if (type != 2)
        PriorityQueue__push(q, current_process, -1 * current_process->t_remaining);
    else
        FIFOQueue__push(q, current_process);
}
////////////////////////////////////////////////////////////
