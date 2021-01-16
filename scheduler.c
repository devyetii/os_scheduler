#include "lib/clock.h"
#include "lib/ipc.h"
#include <stdio.h>
#include <math.h>
#include "lib/io.h"
#include "lib/data_structures.h"
#include "lib/process_management.h"
#include "lib/remaining_time.h"
#include "lib/memory.h"

int sem_id, msq_id, type, finished = -1, old_clk = -1, change_child = 0, run_clk = -1, total_t_w = 0, number_process = 0;
float total_t_WTA = 0;
PCB *current_process = NULL;
bool run = false, p_end = false;
void *Q;
FIFOQueue *t_w;
FILE *schedulerLog;
FILE *memLog;
int total_running = 0;

void intials();
void handlerChild(int signum);
void clearIPC(int signum);
void handlerUSR1(int signum);
bool insert_in_Queue(void *PQ);
bool processNotFinished(void *q, int type);
void runProcess(void *Q, int type);
void finishedProcess();
void STRN(PriorityQueue *Q);
void premtive(void *Q);
void writeInPerf(int lclk);
void writeInLog(int state);
void writeInLogTerminate();

///////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    int quantam;
    type = atoi(argv[1]);
    if (type < 2)
        Q = PriorityQueue__create(10000);
    else
    {
        Q = FIFOQueue__create();
        quantam = atoi(argv[2]);
    }
    intials();
    bool end = false;
    int current_clk;
    while (run || finished < 1 || processNotFinished(Q, type) || !end)
        if (finished > -1)
        {
            current_clk = getClk();
            if (insert_in_Queue(Q) && current_clk > old_clk)
            {
                old_clk = current_clk;
                if (run && type == 1 && processNotFinished(Q, type))
                    STRN(Q);
                if (run && type == 2 && current_clk == run_clk + quantam && current_process->t_remaining != 0)
                {
                    if (!processNotFinished(Q, type))
                    {
                        run_clk = getClk();
                        writeInLog(STOPPED);
                        writeInLog(RESUMED);
                    }
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
                if (!run && finished == 1 && !processNotFinished(Q, type))
                    end = true;
            }
        }
    printf("Last clock = %d\n", old_clk);
    writeInPerf(old_clk);
    destroyRemainingTimeCommunication(true);
    destroyClk(false);
    return 0;
}

//////////////////////////function defenations //////////////////////

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
    writeInPerf(getClk());
    destroyRemainingTimeCommunication(true);
    destroyClk(false);
    safeExit(-1);
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
                    p |= (-1 * recievedProcess.t_running) & 0xFFFFFFFF;
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
        if (canAllocate(current_process->p_data.p_size) == 1) {
            printf("I can\n");
            current_process->mem_pair = allocate(current_process->p_data.p_size);
            current_process->t_st = getClk();
            current_process->actual_pid = createChild("./process.out", current_process->t_remaining, current_process->p_data.pid);
            writeInLog(STARTED);
            writeMemLog(memLog, current_process->t_st, current_process, 1 /*allocate*/);
        }
    }
    else
    {
        setRemainingTime(current_process->t_remaining);
        change_child++;
        current_process->state = RESUMED;
        kill(current_process->actual_pid, SIGCONT);
        writeInLog(RESUMED);
        printf("p id %d real %d in %d with rem %d\n", current_process->p_data.pid, current_process->actual_pid, getClk(), current_process->t_remaining);
    }
}

void finishedProcess()
{
    run = false;
    if (current_process != NULL)
    {
        writeInLogTerminate();
        writeMemLog(memLog, getClk(), current_process, 0 /*freed*/);
        deallocate(current_process->mem_pair);
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
        writeInLog(STOPPED);
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
///////////////////////////write in files functions //////////////////////////
void writeInPerf(int lclk)
{
    closeFile(schedulerLog);
    closeFile(memLog);
    FILE *schedulerPerf = openFile("scheduler.perf", "w");
    float utilization = ((float)(total_running + 1) / lclk) * 100.0;
    float average_w = (float)total_t_w / number_process;
    float average_WTA = (float)total_t_WTA / number_process;
    float sum = 0;
    while (!FIFOQueue__isEmpty(t_w))
    {
        float *std = FIFOQueue__pop(t_w);
        sum += (*std - average_WTA) * (*std - average_WTA);
    }
    float std_w = sqrtf(sum);
    writeStats(schedulerPerf, "CPU utilizaton = %.2f \%\n", &utilization);
    writeStats(schedulerPerf, "AVG WTA = %.2f\n", &average_WTA);
    writeStats(schedulerPerf, "AVG Waiting = %.2f\n", &average_WTA);
    writeStats(schedulerPerf, "Std WTA = %.2f\n", &std_w);
    closeFile(schedulerPerf);
}
void writeInLog(int state)
{
    current_process->state = state;
    current_process->t_w = getClk() - (current_process->p_data.t_running - current_process->t_remaining + current_process->p_data.t_arrival);
    writeProcess(schedulerLog, current_process, getClk());
}
void intials()
{
    // Init memory
    initMem();
    schedulerLog = openFile("scheduler.log", "w");
    memLog = openFile("memory.log", "w");
    msq_id = getProcessMessageQueue(KEYSALT);
    sem_id = getSem(KEYSALT);
    initRemainingTimeCommunication(true);
    signal(SIGCHLD, handlerChild);
    signal(SIGINT, clearIPC);
    signal(SIGSEGV, clearIPC);
    signal(SIGUSR1, handlerUSR1);
    t_w = FIFOQueue__create();
    kill(getppid(), SIGUSR1);
}
void writeInLogTerminate()
{
    current_process->t_remaining = 0;
    current_process->state = FINISHED;
    current_process->t_w = getClk() - (current_process->p_data.t_running - current_process->t_remaining + current_process->p_data.t_arrival);
    current_process->t_ta = getClk() - current_process->p_data.t_arrival;
    writeProcess(schedulerLog, current_process, getClk());
    total_t_w += current_process->t_w;
    float nearest = current_process->t_ta / (float)current_process->p_data.t_running;
    total_t_WTA += nearest;
    float *t = (float *)malloc(sizeof(float));
    *t = nearest;
    FIFOQueue__push(t_w, t);
    number_process++;
}