/*
 * Here are all the Data Structures needed in the project
 * @author : Ebrahim Gomaa (HmanA6399)
*/
#include <stdio.h>
#include <cstdlib>


//========== begin ProcessData =================
/*
 * Represents Process data read from file
*/
typedef struct ProcessData
{
    int pid;
    int t_arrival;
    int t_running;
    int priority;
} ProcessData;

/*
 * Creates new ProcessData pointer
*/
ProcessData* ProcessData__create(int pid, int t_arr, int t_run, int prior) {
    ProcessData* pd = (ProcessData*) malloc(sizeof(ProcessData));
    pd->pid = pid;
    pd->t_arrival = t_arr;
    pd->t_running = t_run;
    pd->priority = -1 * prior;
}

//========== end ProcessData =================
