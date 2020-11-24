#include <stdio.h>
#include <stdlib.h>
#include "312.c"
#include "test.c"


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

int product(int row_vector[], int col_vector[], int colnum1, int colnum2){
    int count=0;
    int rowvectorelement1;
    int rowvectorelement2;

    int col1length=col_vector[colnum1+1]-col_vector[colnum1];       //The length that the column parts have in the row_vector depiction
    int col2length=col_vector[colnum2+1]-col_vector[colnum2];
    for (int i=0; i<col1length; i++ ){       //checking every element of the first column
        for(int j=0; j<col2length; j++){     //making a comparison with every element of the second column
            rowvectorelement1=row_vector[i+ col_vector[colnum1]];
            rowvectorelement2=row_vector[j+ col_vector[colnum2]];
            if (rowvectorelement1 == rowvectorelement2){
                count ++;
            }
        }
    }
    return count;
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


    CSCArray* a1=COOtoCSC(stream);
    CSCArray* a2=getCSC(a1);


   
    int* Cvalues=malloc(a2->nz*sizeof(int));        
    int cnumelements=0;       //counting how many elements I have in the Cvalues array.
    int Ccol_vector[a2->M+1];
    Ccol_vector[0]=0;
    int Ccolcounter=0;             //variable to count the number of non zero elements in every column


    int col_length;
    int rownum;
    int colnum;
    int productnum;
    
    //Below is the part where we calculate the expression. Please read the product function comments if you want to comprehend the method.

    //We begin by finding out all the non zero elements of the array

    for(int i=0; i<a2->M; i++){        //checking for every column
    col_length=a2->colvector[i+1]-a2->colvector[i];
        for(int j=0; j<col_length; j++){        //checking for each element of the column. This way I find every non zero element of the matrix.
            rownum=a2->rowvector[a2->colvector[i] +j];
            colnum=i;
            productnum=product(a2->rowvector, a2->colvector, rownum, colnum);
            if(productnum>0){                   //Filling out the elements of the Cvaluesvector
                Cvalues[cnumelements]=productnum;
                cnumelements++;
                Ccolcounter++;
            }
        }
        Ccol_vector[i+1]=Ccolcounter +Ccol_vector[i];
        Ccolcounter=0;
    }
    Cvalues=realloc(Cvalues, cnumelements*sizeof(int));     //Reallocating so that there is no excess memory in heap

    int* triangles=triangle_num(Cvalues, Ccol_vector, a2->M);
    //free
    int total_triangles=0;
    for (int i=0; i<a2->M; i++){
        printf("the triangles of the node are %d\n",triangles[i]);
        total_triangles=total_triangles+triangles[i];
    }
    printf("ta trigona ine %d\n",total_triangles);

    free(Cvalues);
    free(triangles);
    

    return 0;
}