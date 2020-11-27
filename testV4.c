#include <stdio.h>
#include <stdlib.h>
#include "test.c"


int product(int* rowVector, int* colVector, int colNum1, int colNum2){
    int result=0;       //The result of the multiplication of the two vectors
    int rowIndice1;     //The column index of the element examined. We take advantage here of the symmetry of the matrix, so the csc and csr formats are equivalent
    int rowIndice2;     //The row index of the element examined

    int colLength1 = colVector[colNum1+1] - colVector[colNum1]; //Number of nonzero elements in column with index colNum1
    int colLength2=colVector[colNum2+1] - colVector[colNum2];   //Number of nonzero elements in column with index colNum2        
    
    int j=0;

    //O(n) complexity
    for(int i=0;i<colLength1;++i){
        rowIndice1 = rowVector[colVector[colNum1]+i];
        rowIndice2 = rowVector[colVector[colNum2]+j];
        while(rowIndice1>rowIndice2){ //o>x
            j++;
            if(j==colLength2){ //elegxos an exei diatr3ei ola ta stoixeia ths 2hs sthlhs
                //printf("in product: column=%d, row=%d, result=%d reason=a\n", colNum1, colNum2, result);
                return result;
            }
            rowIndice2 = rowVector[colVector[colNum2]+j];
        }
        if(rowIndice1==rowIndice2){ //o=x
            result++;
            j++;
            if(j==colLength2){ //elegxos an exei diatr3ei ola ta stoixeia ths 2hs sthlhs
                //printf("in product: column=%d, row=%d, result=%d reason=b\n", colNum1, colNum2, result);
                return result;
            }
        }
        //if o<x then go to the next loop
    }
    //printf("in product: column=%d, row=%d, result=%d reason=c\n", colNum1, colNum2, result);
    return result;
}

int* triangles2(CSCArray* A){
    int* rowVector = A->rowVector;
    int* colVector = A->colVector;
    int M = A->M;

    int colLength;
    int row;
    int column;
    int* trianglesArray=calloc(M, sizeof(int));
    int vectorProduct;

    for(int i=0; i<M; ++i){
        colLength = colVector[i+1] - colVector[i];
        column = i;
        for(int j=0; j<colLength; ++j){
            row = rowVector[colVector[i]+j];
            //printf("index: %d\n", colVector[i]+j);
            //printf("row=%d, index=%d\n", row, colVector[i]+j);
            if(colVector[row+1]-colVector[row] == 0){ //mhdenikh seira
                continue;
            }
            vectorProduct = product(rowVector, colVector, column, row);
            trianglesArray[i] += vectorProduct;
        }
        trianglesArray[i] /= 2;
        //printf("TranglesArray[%d]=%d\n", i, trianglesArray[i]);
    }
    return trianglesArray;
}

int main(int argc, char* argv[]){

    FILE *stream;      //file pointer to read the given file
    MM_typecode t;     //the typecode structt
    
    if(argc<2){
        printf("Please pass as argument the .mtx file\n");
        exit(0);
    }  

    char* s=argv[1];

    //Checking if the argument is .mtx file
    int nameLength = strlen(s);        //length of the name of the file
    if(!((s[nameLength-1]=='x') && (s[nameLength-2]=='t') && (s[nameLength-3]=='m') && (s[nameLength-4]=='.'))){
        printf("Your argument is not an .mtx file\n");
        exit(0);
    }

    //Opening The file as shown in the command line
    stream=fopen(s, "r");        
    if(stream==NULL){
        printf("could not open file, pass another one\n");
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

    int* trianglesArray = triangles2(cscArray);

    int totalTriangles=0;

    for(int i=0;i<M;++i){
        totalTriangles += trianglesArray[i];
    }
    totalTriangles /= 3;

    free(trianglesArray);

    free(colVector);
    free(rowVector);
    free(cscArray);

    printf("Total triangles = %d\n", totalTriangles);

    return 0;
}