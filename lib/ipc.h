#pragma once
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "data_structures.h"
#include <sys/ipc.h>
#include "safeExit.h"

// MQ defines
#define PATH "path"
#define KEYSALT 3532
#define MAX_MSG_SZ 256
#define QFLAGS 0666 | IPC_CREAT
#define SCHEDULER_TYPE 0xCD

// SHM defines
#define SHMPERMS 04444
#define PROCESS_GENERATOR_FINISHED 0xAA

//======== begin Message Queue methods (specialized for the ProcessData)======
typedef struct pdata_msg {
    long mtype;
    ProcessData pdata;
} pdata_msg;

int getProcessMessageQueue(int key_salt) {
    key_t k = ftok(PATH, key_salt);
    int msgq_id = msgget(k, QFLAGS);
    if (msgq_id == -1) {
        char errtxt[256];
        sprintf(errtxt, "Error in creation, key : %d", k);
        perror(errtxt);
        safeExit(-1);
    }
    return msgq_id;
}

pdata_msg createProcessMessage(int type, ProcessData pdata) {
    pdata_msg msg;
    msg.mtype = type;
    msg.pdata = pdata;
    return msg;
}

void sendProcessMessage(pdata_msg message, int msgq_id) {
    int sent_msg_status = msgsnd(msgq_id, &message, sizeof (message.pdata), !IPC_NOWAIT);
    if (sent_msg_status == -1) {
        perror("Error in sending to queue");
        safeExit(-1);
    }
}

ProcessData recieveProcessMessage(int msgq_id, long typ) {
    pdata_msg recieved_message;
    int msgrcv_status = msgrcv(msgq_id, &recieved_message, MAX_MSG_SZ, typ, IPC_NOWAIT);
    if (msgrcv_status != -1) {
        return recieved_message.pdata;
    }
    return NULL_PROCESS_DATA();
}

void deleteProcessMessageQueue(int msgq_id) {
    if (msgctl(msgq_id, IPC_RMID, NULL) == -1) {
		perror("Message queue could not be deleted.");
		safeExit(-1);
	}

	printf("Message queue was deleted.\n");
}
//======== end Message Queue methods (specialized for the ProcessData)======


//======== begin Shared memory methods ======
int getOrCreateShmID(key_t key) {
    int shmid = shmget(key, sizeof(int), IPC_CREAT | SHMPERMS);
    if (shmid == -1) {
        perror("Error in shmget");
        safeExit(-1);
    }
    return shmid;
}

int getShmID(key_t key) {
    int shmid = -1;
    while ((int)shmid == -1)
    {
        shmid = shmget(key, sizeof(int), SHMPERMS);
    }
}

int* getShmAddr(int shmid) {
    int* shmaddr = (int*) shmat(shmid, (void*) 0, 0);
    if (shmaddr == NULL) {
        perror("Error in shmat");
        safeExit(-1);
    }
    return shmaddr;
}

void releaseShmAddr(int* shm_addr) {
    if(shmdt(shm_addr) == -1) {
        perror("Error in shmdt");
        safeExit(-1);
    }
}

void deleteShm(int shm_id) {
    if (shmctl(shm_id, IPC_RMID, (struct shmid_ds*) 0) == -1) {
        perror("Shared Memory couldn't be deleted");
        safeExit(-1);
    }
	printf("Shared Memory was deleted.\n");
}
//======== end Shared memory methods ========