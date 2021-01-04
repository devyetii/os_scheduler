#pragma once
#include <stdlib.h>
#include <stdio.h>

#include "data_structures.h"

#define LINESZ 20

FILE* openFile(char* path, char* perms) {
    FILE* fptr = fopen(path, perms);
    if (fptr == NULL) {
        perror("Error in fopen: ");
        exit(-1);
    }

    return fptr;
}

void closeFile(FILE* fptr) {
    fclose(fptr);
}


ProcessData* readProcess(FILE* f) {
    size_t size = LINESZ;
    char* line = (char*) malloc(LINESZ);

    // Read line and ignore comments 
    do
    {
        getline(&line, &size, f);
    } while ((line[0] != '#') && !feof(f));
    
    int pid, arr, runn, prior;
    sscanf(line, "%d\t%d\t%d\t%d\n", &pid, &arr, &runn, &prior);
    ProcessData* pd = ProcessData__create(pid, arr, runn, prior);
    free(line);

    return pd;
}

void writeProcess(FILE* f, PCB* p, int timestep) {
    printf("writeProcess is not implemented yet\n");    
}

void writeStats(FILE* f, char* stat_name, char* stat_value) {
    printf("writeStats is not implemented yet\n");
}

bool isEndOfFile(FILE* f) {
    return feof(f);
}