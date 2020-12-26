#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
//#include <stdio.h>
#include <sys/wait.h>
int number_process;
int* id;
int* arr;
int* run;
int* prio;

int size_of_file(char* directory)
{
int size=0;
FILE *in1 = fopen(directory, "r");  
int ch=0;
   if (in1 != NULL) 
    {

        while (!feof(in1)) 
        {

	ch=fgetc(in1);
        if(ch=='\n')
	{
		size++;
	}		
        }
        
        fclose(in1);
    }
    else{return 0;}

return size;
}

void read_IO(char* directory)
{

int s=size_of_file("t.txt");
id=calloc(s,sizeof(int));
arr=calloc(s,sizeof(int));
run=calloc(s,sizeof(int));
prio=calloc(s,sizeof(int));
int ids,at,rt,p;
char comment[BUFSIZ];
int g=0;
FILE *in = fopen(directory, "r");  

   if (in != NULL) 
    {
        char line[BUFSIZ];
        while (!feof(in)) 
        {
    
        fgets(comment,BUFSIZ,in);
        if(comment[0]=='#')
	{
	
	continue;
	}
            			
	    sscanf(comment, "%d\t%d\t%d\t%d\n", &ids, &at, &rt, &p);
            //printf("\n%d\t%d\t%d\t%d  " ,ids,at,rt,p);	     
	     id[g]=ids;
	     arr[g]=at;
	     run[g]=rt;
	     prio[g]=p;

	     	g++;
        }
        g--;
    number_process=g;
        fclose(in);
    }

}
void write_IO(int* time,int* id,char* *status,int* arr,int* run,int* remain,int* wait,int* TA,float*  WTA,int n,int fn,char* dir,float cpu,float Avg_WTA,float Avg_waiting,float Std_WTA,char* dir1)
{

FILE *in = fopen(dir, "w");
fprintf(in,"#At time x process y state arr w total z rmain y wait k\n");  
int k=0;
for(int i=0;i<n;i++)
{
if(status[i]=="finished")
{
fprintf(in,"At time %d process %d %s arr %d total %d rmain %d wait %d TA %d WTA %f\n",time[i],id[i],status[i],arr[i],run[i],remain[i],wait[i],TA[k],WTA[k]);  
k++;
}
else
{
fprintf(in,"At time %d process %d %s arr %d total %d rmain %d wait %d\n",time[i],id[i],status[i],arr[i],run[i],remain[i],wait[i]);  
}
}
fclose(in);
//////////////////////////*****************//////////////////
FILE *in1 = fopen(dir1, "w");
fprintf(in1,"CPU utalization =  %f% \nAvg WTA =  %f\nAvg Waiting =  %f\nStd WTA = %f",cpu,Avg_WTA,Avg_waiting,Std_WTA);
fclose(in1);
}



int main()
{
read_IO("t.txt");
///////////////////****************/////////////////
char* dir1="output1.txt";//directory of the file
char* dir2="output2.txt";//directory of the file
const int n=6;
const int fn=2;
int time[6]={1,3,3,6,6,10};
int ids[6]={1,1,2,2,1,1};
char* status[6]={"started","stopped","started","finished","resumed","finished"};
int arrt[6]={1,1,3,3,1,1};
int runt[6]={6,6,3,3,6,6};
int remain[6]={6,4,3,0,4,0};
int wait[6]={0,0,0,0,3,3};
int TA[2]={3,10};
float WTA[2]={1,1.67};
float cpu=100;
float avg_wta=1.34;
float avg_waiting=1.5;
float std_wta=0.34;
/////////*******************/////////
write_IO(time,ids,status,arrt,runt,remain,wait,TA,WTA,n,fn,dir1,cpu, avg_wta,avg_waiting,std_wta,dir2);

/////////////////for us only /////////////////
int g=number_process;
    printf("\n g=%d",number_process);
    printf("\n ");
    for(int y=0;y<g;y++)
    {
    printf("%d ", id[y]);
    }
printf("\n ");

    for(int y=0;y<g;y++)
    {
    printf("%d ", arr[y]);
    }
    printf("\n ");
    for(int y=0;y<g;y++)
    {
    printf("%d ", run[y]);
    }
    printf("\n ");
    for(int y=0;y<g;y++)
    {
    printf("%d ", prio[y]);
    }
    printf("\n ");


return 0;
}
