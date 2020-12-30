#include "lib/data_structures.h"

/**
 * Try FIFOQueue with 1000 ProcessData
*/
void tryFIFO() {
    FIFOQueue* fq = FIFOQueue__create();

    ProcessData *x;
    for (size_t i = 0; i < 1000; i++)
    {
        x = (ProcessData*) malloc(sizeof(ProcessData));
        x->pid = i;
        FIFOQueue__push(fq, x);
    }

    printf("pid\n");
    while (!FIFOQueue__isEmpty(fq))
    {
        x = FIFOQueue__pop(fq);
        printf("%d\n", x->pid);
        free(x);
    }
}

/**
 * Try PriorityQueue with 1000 ProcessData instantiated with randomized priority
*/
void tryPrior() {
    PriorityQueue* pq = PriorityQueue__create(10000);

    printf("pq @ %d size %d max %d\n", pq, sizeof(pq->heap), pq->size);

    ProcessData *x;
    for (size_t i = 0; i < 1000; i++)
    {
        x = (ProcessData*) malloc(sizeof(ProcessData));
        x->pid = i; x->priority = (rand() % 10000) * -1;
        PriorityQueue__push(pq, x, x->priority);
    }
    
    printf("pid\tpriority\n");
    while (!PriorityQueue__isEmpty(pq)) {
        x = PriorityQueue__pop(pq);
        printf("%d\t%d\n", x->pid, x->priority);
        free(x);
    }

    PriorityQueue__destroy(pq);
}

int main(int argc, char const *argv[])
{
    // Try whichever you want
    return 0;
}
