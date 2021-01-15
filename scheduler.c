#include "lib/clock.h"
#include "lib/ipc.h"
#include "lib/data_structures.h"
#include "lib/process_management.h"
#include "lib/remaining_time.h"
int sem_id, msq_id, type, finished = -1, old_clk = -1, change_child = 0, run_clk = -1;
PCB *current_process = NULL;
bool run = false, p_end = false;
void *Q;

void handlerChild(int signum);
void clearIPC(int signum);
void handlerUSR1(int signum);
bool insert_in_Queue(void *PQ);
bool processNotFinished(void *q, int type);
void runProcess(void *Q, int type);
void finishedProcess();
void STRN(PriorityQueue *Q);
void premtive(void *Q);
int main(int argc, char *argv[])
{
    printf("scheduar id %d \n", getpid());

    int quantam;
    type = atoi(argv[1]);
    if (type < 2)
        Q = PriorityQueue__create(10000);
    else
    {
        Q = FIFOQueue__create();
        quantam = atoi(argv[2]);
    }
    msq_id = getProcessMessageQueue(KEYSALT);
    sem_id = getSem(KEYSALT);
    initRemainingTimeCommunication(true);
    signal(SIGCHLD, handlerChild);
    signal(SIGINT, clearIPC);
    signal(SIGUSR1, handlerUSR1);
    kill(getppid(), SIGUSR1);
    while (1)
    {
        if (finished > -1)
        {
            int current_clk = getClk();
            if (insert_in_Queue(Q) && current_clk > old_clk)
            {
                old_clk = current_clk;
                if (run && type == 1 && processNotFinished(Q, type))
                    STRN(Q);
                if (run && type == 2 && current_clk == run_clk + quantam && current_process->t_remaining != 0)
                {
                    if (!processNotFinished(Q, type))
                        run_clk = getClk();
                    else
                        premtive(Q);
                }
                if (!run && processNotFinished(Q, type))
                    runProcess(Q, type);
                if (current_process != NULL)
                {
                    current_process->t_remaining--;
                    p_end = false;
                }
                while (getClk() == old_clk)
                    if (p_end && current_process != NULL)
                    {
                        current_process->t_remaining--;
                        p_end = false;
                    }
            }
        }
    }
    destroyRemainingTimeCommunication(true);
    destroyClk(false);
    return 0;
}

void handlerChild(int signum)
{
    if (change_child == 0)
        finishedProcess();
    else
        change_child--;
    signal(SIGCHLD, handlerChild);
}

void clearIPC(int signum)
{
    destroyRemainingTimeCommunication(true);
    destroyClk(false);
    exit(0);
    signal(SIGINT, clearIPC);
}

void handlerUSR1(int signum)
{
    finished++;
    if (finished == 0)
        initClk();
    signal(SIGUSR1, handlerUSR1);
}

bool insert_in_Queue(void *PQ)
{
    int f = finished;
    int sem_state = __down(sem_id);
    if (sem_state != -1)
    {
        int pid = 0;
        while (pid != -1)
        {
            ProcessData recievedProcess = recieveProcessMessage(msq_id, SCHEDULER_TYPE);
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
        return true;
    }
    if (f == 1 && sem_state == -1)
        return true;
    return false;
}

bool processNotFinished(void *q, int type)
{
    if (type != 2)
        return !PriorityQueue__isEmpty(q);
    return !FIFOQueue__isEmpty(q);
}

void runProcess(void *Q, int type)
{
    if (type < 2)
        current_process = PriorityQueue__pop(Q);
    else
        current_process = FIFOQueue__pop(Q);
    run = true;
    run_clk = getClk();
    if (current_process->actual_pid == -1)
    {
        current_process->t_st = getClk();
        current_process->actual_pid = createChild("./process.out", current_process->t_remaining, current_process->p_data.pid);
    }
    else
    {
        setRemainingTime(current_process->t_remaining);
        change_child++;
        kill(current_process->actual_pid, SIGCONT);
        printf("p id %d real %d in %d with rem %d\n", current_process->p_data.pid, current_process->actual_pid, getClk(), current_process->t_remaining);
    }
}

void finishedProcess()
{
    run = false;
    if (current_process != NULL)
    {
        current_process->t_remaining = 0;
        printf("Process %d real %d finished at %d\n", current_process->p_data.pid, current_process->actual_pid, getClk());
        PCB__destroy(current_process);
        current_process = NULL;
        p_end = true;
        if (processNotFinished(Q, type))
            runProcess(Q, type);
    }
}
void STRN(PriorityQueue *Q)
{
    if (current_process != NULL)
    {
        PCB *process = PriorityQueue__peek(Q);
        //printf("new process %d current process %d \n", process->t_remaining, current_process->t_remaining);
        if (process->t_remaining < current_process->t_remaining)
        {
            premtive(Q);
        }
    }
}
void premtive(void *Q)
{
    if (current_process != NULL)
    {
        change_child++;
        run = false;
        current_process->state = STOPPED;
        // set another data in output file
        printf(" process %d stopped at %d \n", current_process->p_data.pid, getClk());
        if (type != 2)
            PriorityQueue__push(Q, current_process, -1 * current_process->t_remaining);
        else
            FIFOQueue__push(Q, current_process);
        int pid = current_process->actual_pid;
        current_process = NULL;
        kill(pid, SIGSTOP);
        if (processNotFinished(Q, type))
            runProcess(Q, type);
    }
}