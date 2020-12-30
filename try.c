#include "lib/data_structures.h"

int main(int argc, char const *argv[])
{
    FIFOQueue* fq = FIFOQueue__create();
    ProcessData * x1 = (ProcessData *) malloc(sizeof(ProcessData)); x1->pid = 2200;
    FIFOQueue__push(fq, x1);
    ProcessData * x2 = (ProcessData *) malloc(sizeof(ProcessData)); x2->pid = 25654;
    FIFOQueue__push(fq, x2);

    ProcessData *y, *p;
    while(FIFOQueue__isEmpty(fq) != 1) {
        p = (ProcessData *) (FIFOQueue__peek(fq));
        printf("Peek %d\n", p->pid);
        y = (ProcessData *) (FIFOQueue__pop(fq));
        printf("popped %d\n", y->pid);
    }
    return 0;
}
