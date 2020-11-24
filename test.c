//Source file containing functions that help us take an mtx file and take the associated CSC format of the array

#include <stdio.h>
#include "mmio.c"
#include <string.h>

/** Below are the type definition of a struct, which resembles the CSC data structure (but has no values vector because they are all=1)
 * There is also a function that frees the 2 dynamically allocated vectors of the structure.-
 * */

typedef struct{            
    int* rowvector;
    int* colvector;
    int nz;
    int M;
} CSCArray;


void CSCArrayfree(CSCArray* arg){
    free(arg->colvector);
    free(arg->rowvector);
}




/** The following function takes a FILE* associated with an mtx file (with the characteristics that are wanted in this project) and 
 * returns a CSC structure. The returned structure is only the lower triangular part of the symmetric matrix!!
 * There are other functions written in the same file that convert this CSC structure to a CSC structure of the complete matrix.-
 * */

CSCArray* COOtoCSC(FILE* stream){        

    //aquiring data about the sizes
    int M,N,nz;
    
    mm_read_mtx_crd_size(stream, &M, &N, &nz);

    //finding out how many digits the M number is comprised of (used to create the buffer)
    int Mdigits=0;
    int count=M;
    while (count != 0) {
        count /= 10;     // n = n/10
        ++Mdigits;
    }

    
    //doing the conversion
    char buffer[Mdigits+1];        //buffer used to process the data from each line
    int* colvector=malloc((M+1)*sizeof(int));         //vectors that comprise the data structure of the CSC format
    colvector[0]=0;
    int* rowvector=malloc(nz*sizeof(int));

    int temp1,temp2,temp3;
    int colcheck=M;      //the integer used to understand whether we have finished converting a column. The initial value is M(the biggest possible) 
    int count2=0;           //counter that will resemble the number of values in each tested column
    int colindex=1;         //integer used as the index for the filling of the colvector 
    
   
    for(int i=0; i<nz; i++){        //the loop that will fill out the vectors
        fscanf(stream,"%s",buffer);
        temp1=atoi(buffer)-1;
        fscanf(stream,"%s",buffer);
        temp2=atoi(buffer)-1;
        rowvector[i]=temp1;
        
        if(colcheck<temp2){            //Statement that decides whether to fill out a value in the colvector or not
            if(temp2-colcheck>1){       //Checking for the special event when one column has only elements equal to zero
                temp3=colvector[colindex-1]+count2;
                for (int k=0; k<(temp2-colcheck); k++){
                    colvector[colindex]=temp3;
                    colindex++;
                }
                count2=0;
            }
            else{
            colvector[colindex]=colvector[colindex-1] + count2;
            colindex++;
            count2=0;
            }
        }

        count2++;
        colcheck=temp2;
    }
    colvector[colindex]=colvector[colindex-1]+count2;       
    while(colindex<M){          //Filling the last values, in case there are only zero elements from one column one
        colindex++;
        colvector[colindex]=colvector[colindex-1];
    }  
    
    CSCArray* retval=malloc(sizeof(CSCArray));      //creating the value to be returned
    retval->colvector=colvector;
    retval->rowvector=rowvector;
    retval->M=M;
    retval->nz=nz;
    free(stream);     
    return retval;  
}


/** This function takes as a parameters the vectors of an Upper triangular CRS matrix and the number of a specific column. 
 * (eg if column_num==3 then we study the third column)
 *  It returns another vector, which tells us which rows of the CRS on this specific column are non zero. The first element of this array has the 
 * special property that it shows the number of the elements.
 * 
 * e.g. if retarray[0]=, retarray[1]=1 and retarray[3]=2, then we know that we have two non zero elements in the upper triangular part of this clumn of the array, 
 * that are in the second and third row
 * 
 * Useful when trying to calculate the CSC format of the full Matrix, but only have the lower triangular part.-
 * */

int* non_zero_rows(int* row_vector, int* col_vector, int column_num){

    int row_length;
    int* retarray=malloc((column_num+1)*sizeof(int));       //Creating the returned value
    int element_count=0;
    
    for( int i=0; i<column_num; i++){            //checking every row (number of rows=column_num)
        row_length= row_vector[i+1]- row_vector[i];
        for (int j=0; j<row_length; j++){
            if( col_vector[row_vector[i] +j]==column_num){
                element_count++;
                retarray[element_count]=i;
            }
        }
    }
    
    retarray[0]=element_count;
    return retarray;
}       


/** Below is a function that takes the CSC structure that is created by the COOtoCSC function, which is the representation of a lower 
 * triangular matrix, and computes and returns the CSC of the complete matrix (including the symmetric/upper triangular part).-
 * */

CSCArray* getCSC(CSCArray* csc_initial){
    
    int* rowvector_final=malloc(2*csc_initial->nz*sizeof(int));
    int rowvector_count=0;
    int* colvector_final=malloc((csc_initial->M+1) *sizeof(int));
    int colvector_count=0;
    int csc_initial_nonzerosnum;          //the number of non zero elements in every column of the initial lower triangular matrix
    

    int *upper_values;
    
    int* (*upper_nonzeros) (int row_vector[], int col_vector[], int column_num);        //using function pointer to use the upper_non_zeros function
    upper_nonzeros=non_zero_rows;

    for(int i=0; i<csc_initial->M; i++){        //doing the procedure for every column
        
        //getting the upper triangular values
        upper_values=upper_nonzeros(csc_initial->colvector, csc_initial->rowvector, i);
        if(upper_values[0]>0){
            
            for(int j=0; j<upper_values[0]; j++){
                rowvector_final[rowvector_count]=upper_values[j+1];
                rowvector_count ++;
                colvector_count ++;
            }
            
            
        free(upper_values);
        }
        
        else{
            free(upper_values);
        }

        //getting the lower triangular values
        if(i<csc_initial->M-1){
            csc_initial_nonzerosnum=csc_initial->colvector[i+1] - csc_initial->colvector[i];
            for (int j=0; j< csc_initial_nonzerosnum; j++){             //getting the values from the lower triangular
                rowvector_final[rowvector_count]=csc_initial->rowvector[csc_initial->colvector[i] +j];
                rowvector_count ++;
                colvector_count ++;
            }
        }
        colvector_final[i+1]=colvector_final[i]+colvector_count;
        colvector_count=0;
    }    
    //filling the returned value and freeing the memory allocated by the initial CSC data structure
    CSCArray* ret=malloc(sizeof(CSCArray));
    ret->colvector=colvector_final;
    ret->rowvector=rowvector_final;
    ret->M=csc_initial->M;
    ret->nz=rowvector_count;
    CSCArrayfree(csc_initial);
    free(csc_initial);
    return ret;
}
