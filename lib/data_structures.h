/*
 * Here are all the Data Structures needed in the project
 * @author : Ebrahim Gomaa (HmanA6399)
*/
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "safeExit.h"

/**
 * Enumerating process state
*/
typedef enum pstate {
    RUNNING,
    STOPPED,
    FINISHED,
    RESUMED,
    STARTED,    
    IDLE
} pstate;

//========= begin basic DS ===========
typedef struct Pair
{
    int lower_bound;
    int upper_bound;
} Pair;

Pair Pair__create(int lower_bound,int upper_bound)
{
    Pair p;
    p.lower_bound = lower_bound;
    p.upper_bound = upper_bound;
    return p;
}

void Pair__print(Pair p) {
    printf("(%d, %d)\n", p.lower_bound, p.upper_bound);
}
//========= e n d basic DS ===========


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
    int p_size;
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
ProcessData* ProcessData__create(int pid, int t_arr, int t_run, int prior, int p_size) {
    ProcessData* pd = (ProcessData*) malloc(sizeof(ProcessData));
    pd->pid = pid;
    pd->t_arrival = t_arr;
    pd->t_running = t_run;
    pd->priority = -1 * prior;
    pd->p_size = p_size;
    return pd;
}

/**
 * Print ProcessData
 * 
 * @param pd Pointer to ProcessData
*/
void ProcessData__print(ProcessData* pd) {
    printf("%d\t%d\t%d\t%d\t%d\n", pd->pid, pd->t_arrival, pd->t_running, pd->priority, pd->p_size);
}

/**
 * Free the allocated memory for given ProcessData pointer
 * 
 * @param pd Pointer to ProcessData memory location
*/
void ProcessData__destroy(ProcessData* pd) {
    if (pd != (ProcessData*) -1 && pd != NULL)
        free(pd);
}

ProcessData NULL_PROCESS_DATA() {
    ProcessData pd;
    pd.pid = -1;
    return pd;
}
//========== end ProcessData =================

//========== begin PCB ======================
/**
 * Representation of the process in the scheduler module
*/
typedef struct PCB
{
    ProcessData p_data;
    int t_remaining;
    int t_ta; // Turn-around time
    int t_w;  // Waiting time
    int t_st;
    int actual_pid;
    pstate state;
    Pair mem_pair;
} PCB;

/**
 * Creates new PCB pointer
 * 
 * @param p_data Pointer to the process data instance
 * @param t_r    Initial Remaining Time
 * @param t_ta   Initial Turnaround time
 * @param t_w    Initial waiting Time
 * @param state  Process State
 * @param actual_pid  Pid of process
 * @param t_st  frist clock in run
 * @return Pointer to the created PCB Instance
*/
PCB *PCB__create(ProcessData p_data, int t_r, int t_w, int t_ta, pstate state, int actual_pid, int t_st)
{
    PCB *pcb = (PCB *)malloc(sizeof(PCB));
    pcb->p_data = p_data;
    pcb->t_remaining = t_r;
    pcb->t_ta = t_ta;
    pcb->t_w = t_w;
    pcb->state = state;
    pcb->t_st = t_st;
    pcb->actual_pid = actual_pid;
    return pcb;
}

/**
 * Free the allocated memory for given PCB pointer
 * 
 * @param pcb Pointer to PCB memory location
*/
void PCB__destroy(PCB* pcb) {
    if (pcb != (PCB*) -1 && pcb != NULL)
        free(pcb);
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
    void* val = fq->head->val;

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
static inline int parent(int i) { return i>>1; }
static inline int leftChild(int i) { return (i<<1); }
static inline int rightChild(int i) { return (i<<1)+1; }
static inline ushort isLeaf(int idx, int sz) { return ((leftChild(idx) > sz)); }

typedef struct PriorityItem {
    void* val;
    long long priority;
} PriorityItem;

void PriorityItem__swap(PriorityItem* pit1, PriorityItem* pit2) {
    PriorityItem* pit_tmp = (PriorityItem*) malloc(sizeof(PriorityItem));
    memcpy(pit_tmp, pit1, sizeof(PriorityItem));
    pit1->val = pit2->val;
    pit1->priority = pit2->priority;

    pit2->val = pit_tmp->val;
    pit2->priority = pit_tmp->priority;

    free(pit_tmp);
}

typedef struct PriorityQueue {
    struct PriorityItem** heap;
    int size;
} PriorityQueue;

void __maxHeapify(PriorityQueue* pq, int idx) {
    if (isLeaf(idx, pq->size)) return;

    PriorityItem *cur = pq->heap[idx], *left = pq->heap[leftChild(idx)], *right = (rightChild(idx) <= pq->size) ? pq->heap[rightChild(idx)] : NULL;
    
    if (
        cur->priority < left->priority ||
        (right != NULL && cur->priority < right->priority)
    ) {
        // Choose the greater for swapping
        PriorityItem* grtr = (right != NULL && (right->priority > left->priority)) ? right : left;
        int grtr_idx = (right != NULL && (right->priority > left->priority)) ? rightChild(idx) : leftChild(idx);
        
        // Swap and recurse
        PriorityItem__swap(cur, grtr);
        __maxHeapify(pq, grtr_idx);
    }
}

PriorityQueue* PriorityQueue__create(int max_sz) {
    PriorityQueue* pq = (PriorityQueue*) malloc(sizeof(PriorityQueue));
    pq->heap = (PriorityItem**) malloc(sizeof(PriorityItem*) * (max_sz + 5));
    pq->heap[0] = (PriorityItem*) malloc(sizeof(PriorityItem));
    pq->heap[0]->val = NULL; pq->heap[0]->priority = __INT64_MAX__;
    pq->size = 0;
    return pq;
}

void ____printHeap(PriorityQueue* pq) {
    printf("\n");
    for (int i = 1; i <= pq->size; ++i) printf("%lld ", pq->heap[i]->priority);
    printf("\n");
}

void PriorityQueue__push(PriorityQueue* pq, void* val, long long prior) {
    // Create the node
    PriorityItem* pit = (PriorityItem*) malloc(sizeof(PriorityItem));
    pit->val = val;
    pit->priority = prior;

    // Place it to the end of the heap
    pq->heap[++pq->size] = pit;

    
    // Fix max heap
    int cur_idx = pq->size;
    while (pq->heap[cur_idx]->priority > pq->heap[parent(cur_idx)]->priority) {
        PriorityItem__swap(pq->heap[cur_idx], pq->heap[parent(cur_idx)]);
        cur_idx = parent(cur_idx);
    }
    // ____printHeap(pq);
}

void* PriorityQueue__peek(PriorityQueue* pq) {
    return pq->heap[1]->val;
}

void* PriorityQueue__pop(PriorityQueue* pq) {
    void* retval = pq->heap[1]->val;

    // Swap head and tail
    pq->heap[1]->val = pq->heap[pq->size]->val;
    pq->heap[1]->priority = pq->heap[pq->size]->priority;

    // Remove tail (old head)
    free(pq->heap[pq->size]);
    pq->size--;

    // Fix heap
    if (pq->size) __maxHeapify(pq, 1);

    return retval;
}

ushort PriorityQueue__isEmpty(PriorityQueue* pq) {
    return pq->size == 0;
}

void PriorityQueue__destroy(PriorityQueue* pq) {
    for (int i = 0; i < pq->size; ++i) { free(pq->heap[i]->val); free(pq->heap[i]); }
    free(pq->heap);
    free(pq);
}
//======= end PriorityQueue ===========================
