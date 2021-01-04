#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "data_structures.h"

FILE* openFile(char* file_name, char permission) {
    printf("openFile is not implemented yet\n");
}

void closeFile(FILE* f) {
    printf("closeFile is not implemented yet\n");
}

ProcessData* readProcess(FILE* f) {
    printf("readProcess is not implemented yet\n");    
}

void writeProcess(FILE* f, PCB* p, int timestep) {
    printf("writeProcess is not implemented yet\n");    
}

void writeStats(FILE* f, char* stat_name, char* stat_value) {
    printf("writeStats is not implemented yet\n");
}

bool isEndOfFile(FILE* f) {
    printf("isEndOfFile is not implemented yet\n");
}