#include <stdio.h>
#include <stdlib.h>
#include "test.c"
#include <cilk/cilk.h>
#include <cilk/cilk_api.h> 
#include <time.h>



/**  The function below (product) is a function that calculates the dot product between two columns.
 * 
 *  It takes as input the matrix in the CSC format (row_vector and col_vector), and the number of each column that takes place in the product.
 *  (e.g. when colnum1=2 and colnum2=3 then the third and the fourth columns will be multiplied)
 * 
 *  Every column corresponds to a specific part of the row_vector. By comparing the two parts, we count how many pairs of equal elements we have
 *  in the two parts, and that is the inner product of the two columns.
 * 
 *  Useful because the matrices that we deal with are symmetric, so the product between a row1 and col2 is equivalent to the dot product between
 *  col1 and col2.
 *  Calculating the product between a rowi and colj is useful because that is the A[i][j] element of the product of two matrices.- 
 * */


int product(int* rowVector, int* colVector, int colNum1, int colNum2){
    int result = 0;
    int smallLength;
    int rowIndice1;     //The column index of the element examined. We take advantage here of the symmetry of the matrix, so the csc and csr formats are equivalent
    int smallColIndex;
    int bigColIndex;
    int bigLength;
    int leftIndex;

    int left;
    int right;
    int middle;
    int flag;

    int colLength1 = colVector[colNum1+1] - colVector[colNum1]; //Number of nonzero elements in column with index colNum1
    int colLength2=colVector[colNum2+1] - colVector[colNum2];

    if(colLength1<=colLength2){
        smallLength = colLength1;
        bigLength = colLength2;
        smallColIndex = colNum1;
        bigColIndex = colNum2;
    }
    else{
        smallLength = colLength2;
        bigLength = colLength1;
        smallColIndex = colNum2;
        bigColIndex = colNum1;
    }

    leftIndex = colVector[bigColIndex];
    right = colVector[bigColIndex+1] - 1;

    for(int i=0; i<smallLength; ++i){
        flag = 0;
        rowIndice1 = rowVector[colVector[smallColIndex]+i];
        left = leftIndex;
        right = colVector[bigColIndex+1] - 1;
        middle = (left+right)/2;
        while(left<=right){
            if(rowVector[middle]==rowIndice1){
                flag = 1;
                leftIndex = middle+1;
                break;
            }
            if(rowVector[middle] < rowIndice1){
                //printf("<\n");
                left = middle+1;
                leftIndex = left;
            }
            else{
                //printf(">\n");
                right = middle-1;
            }
            middle = (left+right)/2;
        }
        //printf("Search result = %d\n", searchResult);
        if(flag == 0){
            continue;
        }
        else{
            result++;
        }
    }

    return result;
}

void compute(int *colVector, int*rowVector, int i, int* triangles){
    int productnum;
    int rowNum;
    for(int j=0; j<colVector[i+1]-colVector[i]; j++){        //checking for each element of the column. This way I find every non zero element of the matrix.
        rowNum = rowVector[colVector[i]+j];
        if(colVector[rowNum+1]-colVector[rowNum] == 0){
            continue;
        }
        productnum=product(rowVector, colVector, i, rowNum);
        if(productnum>0){                   //Filling out the elements of the Cvaluesvector
            triangles[i] += productnum;
        }
    }
    triangles[i]=triangles[i]/2;
}


int main(int argc, char* argv[]){

    FILE *stream;       //creating the stream struct
    MM_typecode t;     //creating the typecode struct
    
    if(argc<2){
        printf("Please pass as argument the .mtx file\n");
        exit(0);
    }  
    char* s=argv[1];

    int a=strlen(s);        //checking if the argument is .mtx file
    if(!( (s[a-1]=='x') && (s[a-2]=='t') && (s[a-3]=='m') && (s[a-4]=='.') )){
        printf("Your argument is not an .mtx file\n");
        exit(0);
    }

    stream=fopen(s, "r");        //opening The file as shown in the command line
    if(stream==NULL){
        printf("could not open file, pass another one\n");
        exit(0);
    }
    mm_read_banner(stream,&t);

    //checking if the matrix type is ok
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


    CSCArray* cscArray = COOtoCSC(stream);
    int* rowVector = cscArray->rowVector;   //The row vector of the sparse matrix in the csc format
    int* colVector = cscArray->colVector;   //The column vector of the sparse matrix in the csc format
    int M = cscArray->M;

    int* triangles=calloc(M, sizeof(int));
    if(triangles==NULL){
        printf("Error in main: Couldn't allocate memory for trianglesArray");
        exit(-1);
    }
  
    //Below is the part where we calculate the expression. Please read the product function comments if you want to comprehend the method.
    if(argc<3){
        printf("Please give me the wanted number of threads as an argument too\n");
        exit(0);
    }

    char* threadNum=argv[2];
    printf("\nYou have chosen %s threads \n",threadNum);

    struct timespec init;
    clock_gettime(CLOCK_MONOTONIC, &init);

    __cilkrts_set_param("nworkers",threadNum);
    #pragma cilk grainsize = 1
    cilk_for (int i=0; i<M; i++){        //checking for every column
        compute(colVector, rowVector, i, triangles);
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

    int total_triangles=0;

    for (int i=0; i<M; i++){
        total_triangles += triangles[i];
    }
    printf("the triangles are %d\n",total_triangles/3);
    
    free(triangles);

    free(colVector);
    free(rowVector);
    free(cscArray);
    
    return 0;
    

    return 0;
}