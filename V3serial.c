/**
 * This program takes a CSC structure and calculates the amount of triangles for each node.
**/

#include <stdio.h>
#include "test.c"
#include <stdbool.h>


/**
 * Function that checks whether a specific column of the matrix (colNum) has an element in a specific row (wantedRow)
 * This is useful in deciding whether A[k][i] != 0 and calculating the number of triangles adjacen to each node
 * Inputs:
 *      int* rowVector: the row indices array of the csc format
 *      int* colVector: the column changes array of the csc format
 *      int colNum: the index of the column we want to examine
 *      int wantedRow: the index of the row that we want our element to belong to
 * Outputs:
 *      int check: 1 if there is an element in (wanted row, colNum), 0 otherwise
 **/

bool elementInColumnCheck(int* rowVector,int* colVector, int colNum, int wantedRow){
    bool check = false;
    int colLength=colVector[colNum +1]-colVector[colNum];
    for (int i=0; i<colLength; i++){
        if (rowVector[colVector[colNum]+i] == wantedRow){
            check = true;
        }
    }
    return check;
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
    
    int* rowVector = cscArray->rowVector;
    int* colVector = cscArray->colVector;
    int M = cscArray->M;

    int elemsInCol;                                 //Number on nonzero elements in a particular column
    int* triangleCount = calloc(M, sizeof(int));    //Each entry contains the number of triangles in which at least an element of this column belongs to
    
    int element1;   //First common we investigate
    int element2;   //Second common indice we investigate

    //Loop that checks for triangles
    //Check for every column
    for(int i=0; i<M; i++){
        elemsInCol = colVector[i+1]- colVector[i];
        //Check for every pair of the column with this double for loop
        for(int j=0; j<elemsInCol; j++){
            element1 = rowVector[colVector[i]+j];
            for (int k=1; k<elemsInCol-j; k++ ){
                element2 = rowVector[colVector[i]+j+k];
                //Check so that we dont compute tha same triangle more than once
                if (element1>i){            
                    //Check if the third common indice exists
                    if (elementInColumnCheck(rowVector, colVector, element1, element2)){
                        triangleCount[element1]++;
                        triangleCount[element2]++;
                        triangleCount[i]++;
                    }
                }      
            }
        }  
    }

    CSCArrayfree(cscArray);
    free(cscArray);

    int totalTriangles=0; //Total number of triangles

    //Compute the total number of triangles
    for (int i=0; i<M; i++){
        printf("to trianglecount einai %d\n",triangleCount[i]);
        totalTriangles += triangleCount[i];
    }
    printf("Total triangles = %d\n",totalTriangles);
   
    free(triangleCount);
    
    return 0;
}

