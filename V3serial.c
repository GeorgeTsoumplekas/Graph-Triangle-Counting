//programm that will take CSC struct and calculate the amount of triangles for each node
#include <stdio.h>
#include "test.c"


/**  The function below takes as paramateres a matrix in CSC format, a specific value (wanted_element) and a specific column number(colnum).
 *  It decides whether this column has this specific element or not. This is useful in deciding whether a A[k][i] !=0 and calculating the number of 
 *  triangles adjacent to each node.-
 * 
 * */

int elementincolumncheck(int row_vector[],int col_vector[], int colnum, int wanted_element){
    int check=0;
    int col_length=col_vector[colnum +1]-col_vector[colnum];
    for (int i=0; i<col_length; i++){
        if (row_vector[col_vector[colnum] +i] == wanted_element )
        check =1;
    }
    return check;
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
    CSCArray* a2=getCSC(a1);        //The full array in CSC
    
    
    int col_length;
    int trianglecount[a2->M];
    for (int i=0; i<a2->M; i++){        //filling with zeros
        trianglecount[i]=0;
    }
    
    int element1;
    int element2;

    for(int i=0; i<a2->M; i++){         //Check for every column
    col_length=a2->colvector[i +1]-a2->colvector[i];
        for(int j=0; j<col_length; j++){        //Check for every pair of the column
            for (int k=1; k< col_length-j; k++ ){
                element1= a2->rowvector[a2->colvector[i] +j];
                element2= a2->rowvector[a2->colvector[i] +j+k];
                
                if (element1>i){            //statement so that we dont compute tha same triangle more than once
                    if (elementincolumncheck(a2->rowvector, a2->colvector, element1, element2 ) >0){
                        trianglecount[element1]++;
                        trianglecount[element2]++;
                        trianglecount[i]++;
                    }
                }   
                
                
            }
        }  
    }

    
    int M=a2->M;
    CSCArrayfree(a2);
    free(a2);

    int total_triangle_num=0;
    for (int i=0; i<M; i++){
        printf("to trianglecount ine %d\n",trianglecount[i]);
        total_triangle_num=total_triangle_num+trianglecount[i];
    }
    printf("to totaltrianglenum ine %d\n",total_triangle_num);
   

    return 0;

}

