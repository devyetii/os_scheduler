#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "data_structures.h"
#include<stdbool.h>
/**
 * readProcess
 * 
 * @param FILE* file
 * @return process
*/


ProcessData* readProcess(FILE* file )
{
        int pids;
        int arrive_time;
        int running_time;
        int priority;
        ProcessData* process;
   if (file != NULL) 
    {
        char line[BUFSIZ];
        bool found=true;
        while (found) 
        {
        fgets(line,BUFSIZ,file);
        if(line[0]=='#')
	{continue;}    			
	sscanf(line, "%d\t%d\t%d\t%d\n", &pids, &arrive_time, &running_time, &priority);
        process=ProcessData__create(pids,arrive_time,running_time,priority);
        found=false;
        }
        //free(line);
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

FILE* openFile(char* file_name,char* permission)
{
   FILE* file = fopen(file_name, permission);
   if (file != NULL) 
    {return file;}
    else
    {return NULL;}
}
/**
 * closeFile
 * 
 * @param FILE* file
*/
void closeFile(FILE* file)
{fclose(file);}
/**
 * writeProcess
 * 
 * @param FILE* file
 * @param PCB* P
 * @param int timestep
*/
void writeProcess(FILE* file,PCB* P,int timestep)
{   
    char state[10];    
   if (file != NULL) 
    {
      if(P->state==2)
        {fprintf(file,"At time %d process %d Finished arr %d total %d remain %d wait %d TA %d WTA %f\n",timestep,P->p_data.pid,P->p_data.t_arrival,P->p_data.t_running,P->t_remaining,P->t_w,P->t_ta,P->t_ta/(float)P->p_data.t_running);}
        else
        {
           if(P->state==0){strcpy(state,"Running");}
           else if(P->state==1){strcpy(state,"Stopped");}     
           else{strcpy(state,"Idle");}     
          fprintf(file,"At time %d process %d %s arr %d total %d remain %d wait %d\n",timestep,P->p_data.pid,state,P->p_data.t_arrival,P->p_data.t_running,P->t_remaining,P->t_w);
        
        }      
     }
}
/**
 * writeStats
 * 
 * @param FILE* file
 * @param char* stat_name
 * @param char* stat_value
*/
void writeStats(FILE* file, char* stat_name, char* stat_value)
{        
   if (file != NULL) 
    {
     fprintf(file,"\n %s =  %s \n",stat_name,stat_value);
     }
}
/**
 * isEndOfFile
 * 
 * @param FILE* file
 * @return bool
*/
bool isEndOfFile(FILE* file)
{        
        if(feof(file)) 
        {return true;}
        else
        {return false;}
}


