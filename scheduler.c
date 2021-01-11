#include "lib/clock.h"
#include "lib/ipc.h"
#include "lib/data_structures.h"
#include "lib/process_management.h"
#include "lib/remaining_time.h"
///////////////////////////// global variables ////////////////
PCB *current_process;
int msg_q_id, finished = 0, t_rem, sem_id, old_clock = -1, sem_id_rt; // type should change with argv
bool flag_in_run = false, p_need_fill = false, set_Data = false, comp = true, chang_in_child = false;
/////////////////// functions handle signals /////////////////
void handlerAndIntial();
void handler(int signum);
void handleProcessFinished(int signum);
//void stopProcess(int sinum);
void clearIPC(int signum);
/////////////////////////function for all algorithim/////////////
void setFinishedProcesss();
bool processNotFinished(void *q, int type);
void insert_all_comming_in_Queue(void *Q, int type);
void setDateNotFinishedP(void *q, int type);
void startProcess(void *PQ, int type);
///////////////////////////////functions for SRTN /////////////////////
void checkComeShortest(void *PQ, int current_child_pid);
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
        insert_all_comming_in_Queue(Queue, type);
        if (!flag_in_run && processNotFinished(Queue, type))
            startProcess(Queue, type);

        if (p_need_fill)
            setFinishedProcesss();
    }
    if (__down(sem_id_rt) != -1)
        printf("done now \n");
    if (flag_in_run && __down(sem_id_rt) != -1)
    {
        printf("here herer herer herer \n");
        t_rem = getReminingTime();
        if (type == 1 && processNotFinished(Queue, type))
            checkComeShortest(Queue, current_process->actual_pid);
        else if (type == 2 && getClk() - old_clock >= quantum && t_rem != 0)
        {
            comp = false;
            set_Data = true;
        }

        if (set_Data)
            setDateNotFinishedP(Queue, type);
    }
    destroyRemainingTimeCommunication(true);
    deleteSemSet(sem_id_rt);
    destroyClk(false);
}

////////////////////////////////function to handle signal /////////////////////
void handlerAndIntial()
{
    initRemainingTimeCommunication(true);
    initClk();
    signal(SIGCHLD, handleProcessFinished);
    signal(SIGUSR1, handler);
    signal(SIGINT, clearIPC);
    msg_q_id = getProcessMessageQueue(KEYSALT);
    sem_id = getSem(KEYSALT);
    sem_id_rt = getSem(RTSEMKEY);
    printf("Recieved queue with id %d\n", msg_q_id);
}

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
void clearIPC(int signum)
{
    destroyRemainingTimeCommunication(true);
    destroyClk(false);
    deleteSemSet(sem_id_rt);
    exit(0);
}
//////////////////// for SRTN ///////////////////////
void checkComeShortest(void *PQ, int current_child_pid)
{
    PCB *process = PriorityQueue__peek(PQ);
    if (process->t_remaining < t_rem)
    {
        comp = false;
        set_Data = true;
    }
}
////////////////////////for HPF SRTN ////////////////////////////////
void startProcess(void *PQ, int type)
{
    insert_all_comming_in_Queue(PQ, type);
    if (type != 2)
        current_process = PriorityQueue__pop(PQ);
    else
        current_process = FIFOQueue__pop(PQ);
    flag_in_run = true;
    comp = true;
    old_clock = getClk();
    t_rem = current_process->t_remaining;
    if (current_process->actual_pid == -1)
    {
        current_process->t_st = getClk();                                                                                      // set all others
        current_process->actual_pid = createChild("./process.out", current_process->t_remaining, current_process->p_data.pid); // anything else process nee
                                                                                                                               // printf("  %d  %d \n", current_process->p_data.pid, current_process->t_remaining);
    }
    else
    {
        chang_in_child = true;
        kill(current_process->actual_pid, SIGCONT);
        //kill(current_process->actual_pid, SIGUSR1);
    }
}

void insert_all_comming_in_Queue(void *PQ, int type)
{
    if (finished != 2 && __down(sem_id) != -1)
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
///////////////////////////////function for all algorithim////////////////////////////////
void setFinishedProcesss()
{
    flag_in_run = false;
    p_need_fill = false;
    printf("Process %d finished at %d\n", current_process->p_data.pid, getClk());
    // TODO :: PRINTING AND STATISTICS
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
    current_process->t_remaining = t_rem;
    printf("Process %d switch at %d\n", current_process->p_data.pid, getClk());
    set_Data = false;
    chang_in_child = true;
    kill(current_process->actual_pid, SIGSTOP);
    flag_in_run = false;
    if (type != 2)
        PriorityQueue__push(q, current_process, -1 * current_process->t_remaining);
    else
        FIFOQueue__push(q, current_process);
}
////////////////////////////////////////////////////////////
