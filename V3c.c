/**
 * This program takes a CSC structure and calculates the amount of triangles for each node.
**/

#include <stdio.h>
#include "test.c"
#include <stdbool.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h> 
#include <pthread.h>
#include <time.h>

//Initialising 3 locks, so that I have one lock for every variable changes be the threads. If I had only one lock and excluded the three commands that 
//change the variables in one excluded area, then the programm would be a little slower
pthread_mutex_t lockI=PTHREAD_MUTEX_INITIALIZER; 


/**
 * Function that checks whether a specific column of the matrix (colNum) has an element in a specific row (wantedRow)
 * This is useful in deciding whether a A[k][i] !=0 and calculating the number of 
 * triangles adjacent to each node.
 * Inputs:
 *      int* rowVector: the row indices array of the csc format
 *      int* colVector: the column changes array of the csc format
 *      int colNum: the index of the column we want to examine
 *      int wantedRow: the index of the row that we want our element to belong to
 * Outputs:
 *      int check: 1 if there is an element in (wanted row, colNum), 0 otherwise
 **/

int elementInColumnCheck(int* rowVector,int* colVector, int colNum, int wantedRow){
    int result=-1;

    int left = colVector[colNum];
    int right = colVector[colNum+1]-1;
    int middle = (left+right)/2;

    while(left<=right){
        if(rowVector[middle]<wantedRow){
            left = middle+1;
        }
        else if(rowVector[middle]==wantedRow){
            result = middle;
            //printf("row=%d, col=%d exists\n", wantedRow, colNum);
            break;
        }
        else{
            right = middle-1;
        }
        middle = (left+right)/2;    
    }
    return result;
}

//Edo ama thes perna san orismata ta colVector kai rowVector katefthian ama thes, den xriazete na pernas ton cscArray aparaitita


void compute(int* rowVector, int* colVector, int* triangleCount, int i){
    int elemsInCol = colVector[i+1]- colVector[i];
    int element1;
    int element2;
    //Check for every pair of the column with this double for loop
    for(int j=0; j<elemsInCol-1; j++){
        element1 = rowVector[colVector[i]+j];
        for (int k=j+1; k<elemsInCol; k++ ){
            element2 = rowVector[colVector[i]+k];            
            //Check if the third common indice exists
            if (elementInColumnCheck(rowVector, colVector, element1, element2)>=0){
                pthread_mutex_lock(&lockI);
                triangleCount[i]++;
                pthread_mutex_unlock(&lockI);
            }
        }      
    }
}

int main(int argc, char* argv[]){
    
    FILE *stream;       //file pointer to read the given file
    MM_typecode t;      //the typecode struct
    
    if(argc<2){
        printf("Please pass as argument the .mtx file\n");
        exit(0);
    }  

    char* s=argv[1];

    //Checking if the argument is .mtx file
    int nameLength = strlen(s);    //length of the name of the file    
    if(!((s[nameLength-1]=='x') && (s[nameLength-2]=='t') && (s[nameLength-3]=='m') && (s[nameLength-4]=='.'))){
        printf("Your argument is not an .mtx file\n");
        exit(0);
    }

    //Opening The file as shown in the command line
    stream=fopen(s, "r");        
    if(stream==NULL){
        printf("Could not open file, pass another file\n");
        exit(0);
    }

    mm_read_banner(stream,&t);

    //Checking if the matrix type is ok
    if (mm_is_sparse(t)==0){
        printf("The array is not sparce. Please give me another matrix market file\n");
        exit(0);
    }
    if (mm_is_coordinate(t)==0){
        printf("The array is not in coordinate format. Please give me another matrix market file\n");
        exit(0);
    }
    if (mm_is_symmetric(t)==0){
        printf("The array is not symmetric. Please give me another matrix market file\n");
        exit(0);
    }

    CSCArray* cscArray = COOtoCSC(stream);  //The sparse array in csc format
    int M = cscArray->M;
    int* rowVector = cscArray->rowVector;
    int* colVector = cscArray->colVector;

    int* triangleCount = calloc(M, sizeof(int));    //Each entry contains the number of triangles in which at least an element of this column belongs to

    if(argc<3){
        printf("Please give me the wanted number of threads as an argument too\n");
        exit(-1);
    }

    char* threadNum=argv[2];
    printf("\nYou have chosen %s threads \n",threadNum);

    struct timespec init;
    clock_gettime(CLOCK_MONOTONIC, &init);

    __cilkrts_set_param("nworkers",threadNum);
    #pragma cilk grainsize = 1
    cilk_for (int i=0; i<M; i++){
        compute(rowVector, colVector, triangleCount, i);
    }

    struct timespec last;   
    clock_gettime(CLOCK_MONOTONIC, &last);

    long ns;
    int seconds;
    if(last.tv_nsec <init.tv_nsec){
        ns=init.tv_nsec - last.tv_nsec;
        seconds= last.tv_sec - init.tv_sec -1;
    }

    if(last.tv_nsec >init.tv_nsec){
        ns= last.tv_nsec -init.tv_nsec ;
        seconds= last.tv_sec - init.tv_sec ;
    }
    printf("The seconds elapsed are %d and the nanoseconds are %ld\n",seconds, ns); 

    CSCArrayfree(cscArray);
    free(cscArray);

    int totalTriangles=0; //Total number of triangles

    //Compute the total number of triangles. I do this by applying a reduction, so that it is computed faster
    
    for (int i=0; i<M; i++){
        totalTriangles += triangleCount[i];
    }
    printf("Total triangles = %d\n",totalTriangles/3);
   
    free(triangleCount);
    
    return 0;
}

