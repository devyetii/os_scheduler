#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "data_structures.h"
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include "memory.h"
/**
 * readProcess
 * 
 * @param FILE* file
 * @return process
*/

ProcessData *readProcess(FILE *file)
{
  int pids;
  int arrive_time;
  int running_time;
  int priority;
  int psize;
  ProcessData *process = NULL;
  if (file != NULL)
  {
    char line[BUFSIZ];
    bool found = true;
    while (found && fgets(line, BUFSIZ, file) != NULL)
    {
      if (line == "") break;
      if (line[0] == '#' || line[0] == '\n')
      {
        continue;
      }
      sscanf(line, "%d\t%d\t%d\t%d\t%d\n", &pids, &arrive_time, &running_time, &priority, &psize);
      process = ProcessData__create(pids, arrive_time, running_time, priority, psize);
      found = false;
    }
    return process;
  }
  return NULL;
}

/**
 * openFile
 * 
 * @param char* file_name
 * @return FILE
*/

FILE *openFile(char *file_name, char *permission)
{
  FILE *file = fopen(file_name, permission);
  if (file != NULL)
  {
    return file;
  }
  else
  {
    return NULL;
  }
}
/**
 * closeFile
 * 
 * @param FILE* file
*/
void closeFile(FILE *file)
{
  fclose(file);
}
/**
 * writeProcess
 * 
 * @param FILE* file
 * @param PCB* P
 * @param int timestep
*/
void writeProcess(FILE *file, PCB *P, int timestep)
{
  char state[10];
  if (file != NULL)
  {
    if (P->state == FINISHED)
    {
      fprintf(file, "At time %d process %d Finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", timestep, P->p_data.pid, P->p_data.t_arrival, P->p_data.t_running, P->t_remaining, P->t_w, P->t_ta, P->t_ta / (float)P->p_data.t_running);
    }
    else
    {
      if (P->state == STOPPED)
      {
        strcpy(state, "Stopped");
      }
      else if (P->state == RESUMED)
      {
        strcpy(state, "Resumed");
      }
      else if (P->state == STARTED)
      {
        strcpy(state, "started");
      }
      else
      {
        strcpy(state, "Idle");
      }
      fprintf(file, "At time %d process %d %s arr %d total %d remain %d wait %d\n", timestep, P->p_data.pid, state, P->p_data.t_arrival, P->p_data.t_running, P->t_remaining, P->t_w);
    }
  }
}

/**
 * @brief Print Memory log line
 * 
 * @param f memory.log file ptr
 * @param time current time
 * @param p PCB of the process
 * @param type Either allocate (1) of free (0)
 */
void writeMemLog(FILE* f, int time, PCB* p, bool type) {
  Pair interval = getMemIntervals(p->mem_pair);
  fprintf(f, "At time %d %s %d bytes for process %d from %d to %d\n", 
    time,
    type ? "allocated" : "freed", 
    p->p_data.p_size,
    p->p_data.pid,
    interval.lower_bound,
    interval.upper_bound
  );
}

/**
 * writeStats
 * 
 * @param FILE* file
 * @param char* stat_name
 * @param char* stat_value
*/
void writeStats(FILE *file, char *stat_name, float *stat_value)
{
  if (file != NULL)
    if (stat_value != NULL)
      fprintf(file, stat_name, *stat_value);
    else
      fprintf(file, stat_name, "%");
}
/**
 * isEndOfFile
 * 
 * @param FILE* file
 * @return bool
*/
bool isEndOfFile(FILE *file)
{
  if (feof(file))
  {
    return true;
  }
  else
  {
    return false;
  }
}
