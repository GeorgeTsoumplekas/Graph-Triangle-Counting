/**
 * 
**/

#include <stdio.h>
#include <stdlib.h>
#include "test.c"

/** 
 *  The function below (product) is a function that calculates the dot product between two columns.
 *  Useful because the matrices that we deal with are symmetric, so the product between a row1 and col2 is equivalent to the dot product between
 *  col1 and col2.
 *  Calculating the product between a rowi and colj is useful because that is the A[i][j] element of the product of two matrices.
 *  Input:
 *      int* rowVector: the row indices array of the csc format
 *      int* colVector: the column changes array of the csc format
 *      int colNum1: the index of the first column used in the multiplication
 *      int colNum2: the index of the second column used in the multiplication
 *  Output:
 *      int result: the result of the multiplication of the two vectors
 *
 *  Every column corresponds to a specific part of the rowVector. By comparing the two parts, we count how many pairs of equal elements we have
 *  in the two parts, and that is the inner product of the two columns.
 * */

int product(int* rowVector, int* colVector, int colNum1, int colNum2){
    int result=0;       //The result of the multiplication of the two vectors
    int rowIndice1;     //The column index of the element examined. We take advantage here of the symmetry of the matrix, so the csc and csr formats are equivalent
    int rowIndice2;     //The row index of the element examined

    int colLength1 = colVector[colNum1+1] - colVector[colNum1]; //Number of nonzero elements in column with index colNum1
    int colLength2=colVector[colNum2+1] - colVector[colNum2];   //Number of nonzero elements in column with index colNum2        
    
    //Checking every element of the first column
    for (int i=0; i<colLength1; i++){  
        rowIndice1 = rowVector[colVector[colNum1]+i];
        //Making a comparison with every element of the second column     
        for(int j=0; j<colLength2; j++){     
            rowIndice2 = rowVector[colVector[colNum2]+j];
            //Check if both of the elements multiplied are nonzero
            if (rowIndice1 == rowIndice2){
                result++;
            }
        }
    }
    return result;
}
/** Se sxolio mia mikri alagi tis product. Tha dume ean ginete na miosume tin poliplokotita
 * 
 * 
int product(int row_vector[], int col_vector[], int colnum1, int colnum2){
    int count=0;
    int rowvectorelement1;
    int rowvectorelement2;

    int col1length=col_vector[colnum1+1]-col_vector[colnum1];       //The length that the column parts have in the row_vector depiction
    int col2length=col_vector[colnum2+1]-col_vector[colnum2];
    int flag=0;

    for (int i=0; i<col1length; i++ ){       //checking every element of the first column
    flag=0;
        if(i<col1length/2){
            for(int j=0; j<col2length&&(flag)<1; j++){     //making a comparison with every element of the second column
               // printf("bika sto proto\n");
                rowvectorelement1=row_vector[i+ col_vector[colnum1]];
                rowvectorelement2=row_vector[j+ col_vector[colnum2]];
                if (rowvectorelement1 == rowvectorelement2){
                  //  printf("bika ke afksithke to count sto proto\n");
                    count ++;
                    flag=1;
                }
            }
        }

        
        else{
            for(int j=col2length -1; j>-1 &&(flag)<1; j--){     //making a comparison with every element of the second column
          //  printf("bika sto deftero\n");
                rowvectorelement1=row_vector[i+ col_vector[colnum1]];
                rowvectorelement2=row_vector[j+ col_vector[colnum2]];
                if (rowvectorelement1 == rowvectorelement2){
                   // printf("bika ke afksithke to count sto deftero\n");
                    count ++;
                    flag=1;
                }
            }
        }
        
        
    }
    return count;
}*/

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

    int counterTemp=0;  
    int colLength;      
    int rowNum;         
    int colNum;         
    int productNum;    

    int* triangles=malloc(M*sizeof(int)); 

    
    //Loop to calculate the expression.
    //We begin by finding out all the non zero elements of the array
    for(int i=0; i<M; i++){        //checking for every column
        colLength = colVector[i+1]- colVector[i];
        for(int j=0; j<colLength; j++){        //checking for each element of the column. This way I find every non zero element of the matrix.
            rowNum = rowVector[colVector[i] +j];
            colNum=i;
            productNum=product(rowVector, colVector, rowNum, colNum);
            if(productNum>0){                   //Filling out the elements of the Cvaluesvector
                counterTemp += productNum;
            }
        }
        triangles[i]=counterTemp/2;

        counterTemp=0;
    }

    int totalTriangles=0;   //Total number of triangles

    //Compute the total number of triangles
    for (int i=0; i<M; i++){
        totalTriangles += triangles[i];
    }
    printf("Total triangles =  %d\n",totalTriangles);

    free(triangles);
    
    return 0;
}