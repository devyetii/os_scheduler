/*
 * Here are all the Data Structures needed in the project
 * @author : Ebrahim Gomaa (HmanA6399)
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Enumerating process state
*/
typedef enum pstate {
    RUNNING,
    STOPPED,
    FINISHED,
    IDLE
} pstate;

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

/**
 * Creates new ProcessData pointer
 * 
 * @param pid ID of the process
 * @param t_arr Arrival Time
 * @param t_run Running Time
 * @param prior Priority index
 * @return Pointer to the created ProcessData Instance
*/
ProcessData* ProcessData__create(int pid, int t_arr, int t_run, int prior) {
    ProcessData* pd = (ProcessData*) malloc(sizeof(ProcessData));
    pd->pid = pid;
    pd->t_arrival = t_arr;
    pd->t_running = t_run;
    pd->priority = -1 * prior;

    return pd;
}

//========== end ProcessData =================

//========== begin PCB ======================
/**
 * Representation of the process in the scheduler module
*/
typedef struct PCB
{
    ProcessData* p_data;
    int t_remaining;
    int t_ta;    // Turn-around time
    pstate state;
} PCB;

/**
 * Creates new PCB pointer
 * 
 * @param p_data Pointer to the process data instance
 * @param t_r    Initial Remaining Time
 * @param t_ta   Initial Turnaround time
 * @param state  Process State
 * @return Pointer to the created PCB Instance
*/
PCB* PCB__create(ProcessData* p_data, int t_r, int t_ta, pstate state) {
    PCB* pcb = (PCB*) malloc(sizeof(PCB));
    pcb->p_data = p_data;
    pcb->t_remaining = t_r;
    pcb->t_ta = t_ta;
    pcb->state = state;

    return pcb;
}
//======= end PCB =========================

//======= begin FIFOQueue =========================
/**
 * Node of a linkedlist
*/
typedef struct Node {
    void* val;
    struct Node* next;
} Node;

/**
 * Linkedlist-implemented FIFOQueue
*/
typedef struct FIFOQueue {
    Node* head;
    Node* tail;
} FIFOQueue;

/**
 * Creates a new FIFOQueue. Initializes head and tail with NULL
 * 
 * @return pointer to FIFOQueue Instance
*/
FIFOQueue* FIFOQueue__create() {
    FIFOQueue* fq = (FIFOQueue*) malloc(sizeof(FIFOQueue));
    fq->head = NULL;
    fq->tail = NULL;

    return fq;
}

/**
 * Returns pointer to the value of the `head` of the Queue without removal
 * 
 * @param fq Pointer to FIFOQueue instance to peak
 * @return pointer to val
*/
void* FIFOQueue__peek(FIFOQueue* fq) {
    if (fq->head == NULL) return NULL;
    return fq->head->val;
}

/**
 * Returns pointer to the value of the `head` of the Queue WITH removal. Update head and tail
 * 
 * @param fq Pointer to FIFOQueue instance to peak
 * @return pointer to val
*/
void* FIFOQueue__pop(FIFOQueue* fq) {
    // Case 0 : Empty LL
    if (fq->head == NULL) return NULL;
    void* val = malloc(sizeof(fq->head->val));
    memcpy(val, fq->head->val, sizeof(fq->head->val));

    // Case 1 : last element
    if (fq->head == fq->tail) {
        free(fq->head);
        fq->head = NULL;
        fq->tail = NULL;
        return val;
    }
    
    // Ordinary Case
    Node* tmp = fq->head;
    fq->head = fq->head->next;
    
    free(tmp);
    return val;
}

/**
 * Create Node of val and insert it to the queue. Update tail and head
 * 
 * @param fq Pointer to FIFOQueue instance to peak
 * @param val pointer to new val
*/
void FIFOQueue__push(FIFOQueue* fq, void* val) {
    Node* nd = (Node*) malloc(sizeof(Node));
    nd->val = val;
    nd->next = NULL;
    if (fq->tail != NULL) fq->tail->next = nd;
    fq->tail = nd;
    
    // If the Queue is initially empty
    if (fq->head == NULL) fq->head = nd;
}

/**
 * Return whether the queue is empty
*/
ushort FIFOQueue__isEmpty(FIFOQueue* fq) {
    return (fq->head == NULL);
}
//======= end FIFOQueue ===========================

//======= begin PriorityQueue =========================
typedef struct PriorityItem {
    void* val;
    int priority;
} PriorityItem;

void PriorityItem__swap(PriorityItem* pit1, PriorityItem* pit2) {
    PriorityItem* pit_tmp = (PriorityItem*) malloc(sizeof(pit1));
    memcpy(pit_tmp, pit1, sizeof(pit1));

    pit1->val = pit2->val;
    pit1->priority = pit2->priority;

    pit2->val = pit_tmp->val;
    pit2->priority = pit_tmp->priority;

    free(pit_tmp);
}

typedef struct PriorityQueue {
    PriorityQueue* heap;
    int size;
} PriorityQueue;

PriorityQueue* PriorityQueue__create(int max_sz) {
    PriorityQueue* pq = (PriorityQueue*) malloc(sizeof(PriorityQueue));
    pq->heap = (PriorityItem*) malloc(sizeof(PriorityItem) * max_sz);
    pq->size = 0;
    return pq;
}


//======= end PriorityQueue ===========================
