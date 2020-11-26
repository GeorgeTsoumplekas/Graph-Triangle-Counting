//Source file containing functions that help us take an mtx file and take the associated CSC format of the array

#include <stdio.h>
#include "mmio.c"
#include <string.h>
#include <math.h>

/** 
 * Type definition of a struct, which resembles the CSC data structure 
 * (no values vector needed because all values are equal to 1)
 **/

typedef struct{            
    int* rowVector;     //array containing the row indeices of each nonzero
    int* colVector;     //array containing the index of the elements which start a column of the sparse matrix
    int nz;             //number of nonzero elements
    int M;              //number of columns (=number of rows because the matrix is square)
} CSCArray;

/**
 * Function that frees the memory allocated for the rowVector and colVector of a specific CSCArray structure.
 * Input:
 *      CSCArray* arg: pointer to the CSCArray structure we want to examine
 * Output:
 *      None
 **/
void CSCArrayfree(CSCArray* arg){
    free(arg->colVector);
    free(arg->rowVector);
}

/**
 * This function takes a FILE* associated with an mtx file (with the characteristics that are wanted in this project) and 
 * returns a CSC structure. The .mtx file contains only the coordinates of the nonzero elements of the lower triangular matrix
 * (because the matrix is symmetric). This function takes advantage of the symmetry of the matrix to create a CSC structure containing
 * all the info needed for all nonzero elements of the matrix.
 * Input:
 *      FILE* stream: pointer to the .mtx file cointaining the info of the sparse matrix
 * Output:
 *      CSCArray* retVal: pointer to a CSCArray structure representing the sparse matrix in CSC format
 **/

CSCArray* COOtoCSC(FILE* stream){        

    printf("started converting mtx file lower triangular CSC\n");
    
    //Aquiring data about the sizes
    int M,N,nz;
    
    mm_read_mtx_crd_size(stream, &M, &N, &nz);

    //Finding out how many digits the M number is comprised of (used to create the buffer)
    int Mdigits=0;
    int digitCount=M;
    while (digitCount !=0 ) {
        digitCount /= 10;     // n = n/10
        ++Mdigits;
    }

    char buffer[Mdigits+1];                     //buffer used to process the data from each line
    int* colVector=malloc((M+1)*sizeof(int));   //index of the elements which start a column of A for the lower triangular part of the matrix
    int* rowVector=malloc(nz*sizeof(int));      //row indices of each non zero element for the lower triangular part of the matrix
    colVector[0]=0;         

    int rowIndiceRead;      //the row indice we read at each line of the .mtx file (first number on the line)
    int colIndiceRead;      //the column indice we read at each line of the .mtx file (second number on the line)
    int elemsUntilZeroCols; //temporarily store the number of elements up until the consecutive all-zero columns
    int colCheck=M+1;       //the integer used to understand whether we have finished converting a column. Initialized with M+1 so that it doesn't go inside the if loop the first time
    int lowerColElements=0; //how many nonzero elements we have in a specific column of the lower triangular matrix
    int colIndex=1;         //integer used as the index for the filling of the colVector 
    int upperColElements=0;  //how many nonzero elements we have in a specific column of the upper triangular matrix

    //CHECK AN DOULEVEI SWSTA
    int **upperVectors = malloc(M*sizeof(int*));       //array containing the row indices of each nonzero element in each column for the upper triangular matrix
    
    //Creating the vectors for the upper triangular part of the matrix. The first element of each vector tells us the number of elements of the vector
    for(int i=0; i<M; i++){     
        upperVectors[i]=malloc(sizeof(int));
        upperVectors[i][0]=1;  
    }
   
    //The loop that will fill out rowVector, colVector and upperVectors
    for(int i=0; i<nz; i++){ 

        fscanf(stream,"%s",buffer);
        rowIndiceRead=atoi(buffer)-1;
        fscanf(stream,"%s",buffer);
        colIndiceRead=atoi(buffer)-1;

        //Check for nonzero elements in the main diagonal
        if(rowIndiceRead == colIndiceRead){
            printf("There are elements in the main diagonal. Please give me an mtx without elements in the diagonal.\n");
            printf("The row of the first element in the diagonal is the %d\n",rowIndiceRead);
            exit(0); //ISWS KALYTERA return NULL ??
        }

        rowVector[i]=rowIndiceRead;
        
        if(colCheck<colIndiceRead){             //Check if the column indice we got is bigger than the previous column indice we examined
            if(colIndiceRead-colCheck>1){       //Checking if one or more consecutive columns have only elements equal to zero
                elemsUntilZeroCols = colVector[colIndex-1] + lowerColElements;
                for (int k=0; k<(colIndiceRead-colCheck); k++){
                    colVector[colIndex]=elemsUntilZeroCols;   //For all these all-zero columns put in the respective column array the value of the total elements up until that point
                    colIndex++;
                }
                lowerColElements=0;
            }
            //If the current element is in a different column than the previous one
            else{ 
                colVector[colIndex]=colVector[colIndex-1] + lowerColElements;
                colIndex++;
                lowerColElements=0;
            }
        }

        lowerColElements++;
        colCheck=colIndiceRead;

        //Note: the equivalent of a csc down triangular matrix is a crs upper triangular matrix. This is why use the row indices here as column indices and vice versa.
        upperVectors[rowIndiceRead][0]++;        //Increase the element counter of the vector of the specific column
        upperVectors[rowIndiceRead]=realloc(upperVectors[rowIndiceRead], (upperVectors[rowIndiceRead][0])*sizeof(int));
        upperColElements=upperVectors[rowIndiceRead][0];
        upperVectors[rowIndiceRead][upperColElements-1]=colIndiceRead; //Add in upperVectors the symmetric element of the one we just read from the file stream
    }

    //Last element of the colVector containing the number of elements of the down triangular matrix
    colVector[colIndex]=colVector[colIndex-1]+lowerColElements;     

    //Filling the last values, in case the last columns are all-zero columns 
    while(colIndex<M){           
        colIndex++;
        colVector[colIndex]=colVector[colIndex-1];
    }  

    free(stream);

    int* finalRowVector = malloc(2*nz*sizeof(int));     //row indices of each non zero element for the whole matrix
    int rowVectorCount=0;                               //shows how many row indices we have added in the finalRowVector
    int* finalColVector = malloc((M+1)*sizeof(int));    //index of the elements which start a column of the whole matrix
    int colVectorCount=0;                               //number of nonzero elements in a particular column for the whole sparse matrix
    int cscInitialColElems;                             //number of nonzero elements in the lower triangular part of the matrix

    //The loop that will fill out finalRowVector and finalColVector
    for(int i=0; i<M; i++){
        
        //Getting the values of the upper triangular half of the matrix
        //Check for nonzero elements in the column
        if(upperVectors[i][0]>1){
            //Add these elements on the final row vector
            for(int j=0; j<upperVectors[i][0] -1; j++){
                finalRowVector[rowVectorCount] = upperVectors[i][j+1];
                rowVectorCount ++;
                colVectorCount ++;
            }     
            free(upperVectors[i]);
        }
        else{
            free(upperVectors[i]);
        }

        //Getting the values of the lower triangular half of the matrix
        //Check for nonzero elements in the column
        if(i<M-1){
            cscInitialColElems=colVector[i+1] - colVector[i];
            //Add these elements on the final row vector
            for (int j=0; j< cscInitialColElems; j++){             
                finalRowVector[rowVectorCount] = rowVector[colVector[i] +j];
                rowVectorCount ++;
                colVectorCount ++;
            }
        }
        finalColVector[i+1] = finalColVector[i] + colVectorCount;
        colVectorCount=0;
    }    

    free(upperVectors);
    free(colVector);
    free(rowVector);
    
    //Creating the CSCAraay to be returned
    CSCArray* retVal=malloc(sizeof(CSCArray));
    retVal->colVector=colVector;
    retVal->rowVector=rowVector;
    retVal->M=M;
    retVal->nz=nz;    
    
    return retVal;  
}