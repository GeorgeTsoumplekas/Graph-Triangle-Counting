#include <stdio.h>
#include "mmio.c"
#include <string.h>
#include <math.h>

typedef struct{            
    int* rowVector;
    int* colVector;
    int nz;
    int M;
} CSCArray;

int* nonZeroElementsInEachColumn(CSCArray* cscInitial){
    int M = cscInitial->M;
    int nz = cscInitial->nz;

    int* sumDownTriangular = malloc(M*sizeof(int)); //isws den xreiazetai
    int* sumUpperTriangular = calloc(M, sizeof(int));
    int* nonZeroElementsCount = malloc(M*sizeof(int));
    int temp;

    for(int i=0; i<nz; ++i){
        temp = (cscInitial->rowVector)[i];
        if(temp >= M){
            printf("Error in nonZeroElements function, pointer out of range");
            exit(-10000);
        }
        sumUpperTriangular[temp]++;
    }

    for(int i=0; i<M; ++i){
        sumDownTriangular[i] = (cscInitial->colVector)[i+1] - (cscInitial->colVector)[i];
        nonZeroElementsCount[i] = sumDownTriangular[i] + sumUpperTriangular[i];
    }

    free((void*)sumUpperTriangular);
    free((void*)sumDownTriangular);

    return nonZeroElementsCount;
}

CSCArray* getCSC(CSCArray* cscInitial){
    printf("started converting lower triangular CSC to complete CSC\n");

    int M = cscInitial->M;
    int totalNZ = 2 * cscInitial->nz;

    int *finalRowVector = malloc(2*(cscInitial->nz*sizeof(int)));
    int *finalColVector = malloc((cscInitial->M+1) *sizeof(int));
    int *nzElementsColNum = nonZeroElementsInEachColumn(cscInitial);

    finalColVector[0] = 0;
    for(int i=1; i<=M; ++i){
        finalColVector[i] = finalColVector[i-1] - nzElementsColNum[i-1];
    }

    for(int i=0; i<totalNZ; ++i){
        
        finalRowVector[i] = 
    }
}