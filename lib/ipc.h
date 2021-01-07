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
#define SHMPERMS 0666
#define REM_TIME_SHM 0xAA

// SEM defines
#define SEMPERMS 0666

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

int getShmID(key_t key, short creator) {
    int shm_id = shmget(key, sizeof(int), SHMPERMS | (creator ? IPC_CREAT : 0));
    if (shm_id == -1) {
        if (creator) {
            perror("Error in starting remaining time shared memory initialization");
            safeExit(-1);
        }
        return -1;
    }

    return shm_id;
}

int* getShmAddr(int shmid) {
    return (int*) shmat(shmid, (void*) 0, 0);
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
//======== begin Semaphore set methods ======
union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux-specific) */
};

/**
 * Get Binary Sem set, init all sims to 0. Exits on failure
 * 
 * @param key_salt Salt for the given key
*/
int getSem(int key_salt) {
    int sem_id = semget(ftok(PATH, key_salt), 1, SEMPERMS | IPC_CREAT);

    if (sem_id == -1) {
        perror("Error in semget");
        exit(-1);
    }   

    union semun initer;
    initer.val = 0;
    if (semctl(sem_id, 0, SETVAL, initer) == -1) {
        perror("Error in semget");
        exit(-1);
    }

    return sem_id;
}

/**
 * Attempt to aquire a symaphore
 * 
 * @param sem_set_id identifier for the used symaphore set
*/
int __down(int sem_set_id) {
    struct sembuf p_op;

    p_op.sem_num = 0;
    p_op.sem_op = -1;
    p_op.sem_flg = IPC_NOWAIT;

    return semop(sem_set_id, &p_op, 1);
}

/**
 * Release a symaphore
 * 
 * @param sem_set_id identifier for the used symaphore set
*/
int __up(int sem_set_id) {
    struct sembuf v_op;

    v_op.sem_num = 0;
    v_op.sem_op = 1;
    v_op.sem_flg = IPC_NOWAIT;

    return semop(sem_set_id, &v_op, 1);
}

/**
 * Clear Sem resource (cancelation point)
 * 
 * @param sem_id id for the sem set to be deleted
*/
void deleteSemSet(int sem_id) {
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
		perror("Sem set could not be deleted.");
		exit(-1);
	}

	printf("Sem set was deleted.\n");
}