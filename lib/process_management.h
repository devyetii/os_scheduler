#pragma once
#include <spawn.h> 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <wait.h>
#include "safeExit.h"

extern char** environ;

int createChild(char* path, int arg1, int arg2)
{
    int pid;
    char s1[10], s2[10];
    sprintf(s1, "%d", arg1);
    sprintf(s2, "%d", arg2);
    char* argV1[] = {path, s1, s2, '\0'};
    int chstat = posix_spawnp(&pid, path, NULL, NULL, argV1, NULL);
        
    if (chstat == 0) {
        return pid;
    } else {
        perror("Error in spawn");
        safeExit(-1);
    }
}

void handleChild(int signum) {
    int status;
    pid_t p = wait(&status);
    printf("A child %d has changed state with status %d\n", p, status);
}

int waitForChild(int child_pid) {
    int status;
    if (waitpid(child_pid, &status, 0) == -1) {
        perror("Error in waitpid");
        safeExit(-1);
    }
    return status;
}