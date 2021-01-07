#pragma once
#include "ipc.h"

// Variables for shared memory
int rem_time_shm_id, *rem_time_shmaddr;
const int rt_shm_k = 0xBCD, rt_sem_k = 0xABC;

void initRemainingTimeCommunication(bool creator) {
    // Init shared memory 
    rem_time_shm_id = getShmID(rt_shm_k, creator);

    rem_time_shmaddr = getShmAddr(rem_time_shm_id);    
}

int getReminingTime() {
    if (rem_time_shmaddr != (void *) -1)
        return *rem_time_shmaddr;
    else
        perror("The shared memory ID wasn't created !!");
        safeExit(-1);
}

void setRemainingTime(int val) {
    if (rem_time_shmaddr != (void *) -1)
        *rem_time_shmaddr = val;
}
