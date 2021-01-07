#pragma once
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

// Redifining the exit system call
void safeExit(int status) {
    if (status == -1)
        killpg(getgid(), SIGINT);
    exit(status);
}
