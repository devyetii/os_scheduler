#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include "data_structures.h"

#define MEM_LEVELS 9
#define MAX_NO_OF_ELEMENTS_PER_LEVEL 1024

static inline int _2pow(int n) { return (1 << n); }
static inline int lvlSz(int lvl) { return (1 << (10 - lvl)); }

// MISS. MEMORY !
ushort M[MEM_LEVELS][MAX_NO_OF_ELEMENTS_PER_LEVEL];

int size_of_level[MEM_LEVELS] = {0};

void printMem() {
    for (int i = MEM_LEVELS - 1; i > 3; --i) {
        for (int j = 0; j < lvlSz(i); ++j) printf("%d ", M[i][j]);
        printf("\n");
    }
}

Pair getMemIntervals(Pair mem_pair) {
    char interval[256];
    int level = mem_pair.lower_bound, block_no = mem_pair.upper_bound;
    int block_sz = _2pow(level);
    int l = block_sz * block_no;
    int u = l + (block_sz - 1);
    return Pair__create(l, u);
}

void initMem() {
    memset(M, MEM_LEVELS*MAX_NO_OF_ELEMENTS_PER_LEVEL, 0);
    size_of_level[8] = 4;
    M[8][0] = M[8][1] = M[8][2] = M[8][3] = 1;
}

ushort canAllocate(int sz) {
    int starting_level = (int) ceil(log2(sz));

    for (int i = starting_level; i < MEM_LEVELS; i++)
        if (size_of_level[i] > 0) return 1;
    return 0;
}

Pair allocate(int sz) {
    if (canAllocate(sz)) {
        int level = ceil(log2(sz));

        int i, j = 0;

        // Determine level of division
        for (i = level; i < MEM_LEVELS && (size_of_level[i] == 0); ++i);
        
        // Determine block
        for (; (j < lvlSz(i)) && (M[i][j] == 0); ++j);

        // Divide
        for (; i > level; --i) {
            M[i][j] = 0;
            size_of_level[i]--;
            M[i-1][2*j] = 1;
            M[i-1][2*j+1] = 1;
            size_of_level[i-1] += 2;
            j <<= 1;
        }

        M[i][j] = 0;    // Used
        size_of_level[i]--;
        return Pair__create(i, j);
    } else {
        printf("Bad memory allocation");
        raise(SIGINT);
    }
}

void deallocate(Pair mem_pair) {
    // Delete memory location
    int i = mem_pair.lower_bound, j = mem_pair.upper_bound;
    M[i][j] = 1;
    size_of_level[i]++;

    for (; i < MEM_LEVELS-1; ++i) {
        int j_adj = j & 0x0001 ? j-1 : j+1;
        if (M[i][j] == 1 && M[i][j_adj] == 1) {
            M[i][j] = 0; M[i][j_adj] = 0;
            size_of_level[i] -= 2;
            M[i+1][j >> 1] = 1;
            size_of_level[i+1]++;
        } else {
            break;
        }
        j >>= 1;
    }
}