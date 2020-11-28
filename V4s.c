#include <stdio.h>
#include <stdlib.h>
#include "test.c"


/**  
 *  Function that calculates the dot product between two columns.
 *  Every column corresponds to a specific part of the row_vector. By comparing the two parts, we count how many pairs of equal elements we have
 *  in the two parts, and that is the dot product of the two columns.
 *  Useful because the matrices that we deal with are symmetric, so the product between a row1 and col2 is equivalent to the dot product between
 *  col1 and col2.
 *  Calculating the product between a rowi and colj is useful because that is the A[i][j] element of the product of two matrices.
 *  To calculate the product, we compare the row indices of the nonzero elements of the first column with the row indices of the non zero
 *  elements of the second column. When the two row indices are equal we increase the result by 1.
 *  The for loop is created in such a way that examines the three cases that may occur:
 *  1)row indice of element of col 1 > row indice of element of col 2: then we compare with the row indice of the following nonzero element of column 2
 *    (Special case): if we have compared with all nonzero elements of column 2, loop exits and function returns the product calculated up to that point
 *  2)row indice of element of col 1 = row indice of element of col 2: we increase result by one and continue the comparison with the following elements 
 *    of those that had the same row indices.
 *    (Special case): if the equality occured for the last nonzero element of column 2, then the function ends and returns the dot product calculated up 
 *    to that point.
 *  3)row indice of element of col 1 < row indice of element of col 2: we move on to the next nonzero element of column 1 and repeat the procedure
 *  In this way we make sure that we do not compare the nonzero elements of the columns more times than needed
 *  Worst case scenario: all nonzero elements (nz) of the sparse matrix are concantrated in one column. Then we have complexity Θ(nz).
 *  In general, this algorithm runs in O(nz) time.
 *  Input:
 *      int* rowVector: the row indices array of the csc format
 *      int* colVector: the column changes array of the csc format
 *      int colNum1: the index of the first column
 *      int colNum2: the index of the second column
 *  Output:
 *      int result: the result of the dot product between the column with index colNum1 and the column with index colNum2
 **/


int product(int* rowVector, int* colVector, int colNum1, int colNum2){
    int result=0;       //The result of the multiplication of the two vectors
    int rowIndice1;     //The column index of the element examined. We take advantage here of the symmetry of the matrix, so the csc and csr formats are equivalent
    int rowIndice2;     //The row index of the element examined

    int colLength1 = colVector[colNum1+1] - colVector[colNum1]; //Number of nonzero elements in column with index colNum1
    int colLength2=colVector[colNum2+1] - colVector[colNum2];   //Number of nonzero elements in column with index colNum2        
    
    int j=0;    //Works as counter for the second column

    //Calculation of the dot product
    for(int i=0;i<colLength1;++i){
        rowIndice1 = rowVector[colVector[colNum1]+i];
        rowIndice2 = rowVector[colVector[colNum2]+j];
        //In this case, move on to the next nonzero element of the second column
        while(rowIndice1>rowIndice2){
            j++;
            //Check if we have compared all the nonzero elements of the second column, in this case the function ends
            if(j==colLength2){ 
                return result;
            }
            rowIndice2 = rowVector[colVector[colNum2]+j];
        }
        //In this case, compare nonzero elements that only have bigger row indices than these ones
        if(rowIndice1==rowIndice2){
            result++;
            j++;
            //Check if we have compared all the nonzero elements of the second column, in this case the function ends
            if(j==colLength2){
                return result;
            }
        }
        //If rowIndice1 < rowIndice2 move on to the next nonzero element of the first column
    }

    return result;
}


int main(int argc, char* argv[]){

    FILE *stream;       //file pointer to read the given file
    MM_typecode t;      //the typecode struct
    
    if(argc<2){
        printf("Please pass as argument the .mtx file\n");
        exit(-1);
    }  
    char* s=argv[1];

    //Checking if the argument is .mtx file
    int nameLength = strlen(s);        //length of the name of the file
    if(!((s[nameLength-1]=='x') && (s[nameLength-2]=='t') && (s[nameLength-3]=='m') && (s[nameLength-4]=='.'))){
        printf("Your argument is not an .mtx file\n");
        exit(-1);
    }

    //Opening The file as shown in the command line
    stream=fopen(s, "r");        
    if(stream==NULL){
        printf("could not open file, pass another one\n");
        exit(-1);
    }

    mm_read_banner(stream,&t);

    //Checking if the matrix type is ok
    if (mm_is_sparse(t)==0){
        printf("The array is not sparce. Please give me another matrix market file\n");
        exit(-1);
    }
    if (mm_is_coordinate(t)==0){
        printf("The array is not in coordinate format. Please give me another matrix market file\n");
        exit(-1);
    }
    if (mm_is_symmetric(t)==0){
        printf("The array is not symmetric. Please give me another matrix market file\n");
        exit(-1);
    }

    CSCArray* cscArray = COOtoCSC(stream); //The sparse array in csc format

    /**
     * The following part of the code calculates the number of triangles adjacent to each node and ultimately the total number of triangles of the sparse matrix
     * Algorithm works as it follows:
     * First of all, we check where we have nonzero elements in A. Let's denote such an element with A[i][j]. Then we calculate the dot product of row with index i
     * with the column of index j. That is because, in the rest of the positions of the matrix there is no need to calculate the dot product due to the fact that we
     * will then multiply it with 0 (since we take the Hadamard product of A with AxA). F
     * Then, multiplication of c with vector e means that we end up with a Mx1 matrix where each row is the sum of the elements of the corresponding column.
     * For this reason, in our algorithm, after we have computed the dot product of row j with column i we add this to trianglesArray[i].
     * To compute the total number of triangles we have to add all triangles adjacent to each node i and then divide it by 3 because each triangle is calculated 3 times.
    **/

    int* rowVector = cscArray->rowVector;   //The row vector of the sparse matrix in the csc format
    int* colVector = cscArray->colVector;   //The column vector of the sparse matrix in the csc format
    int M = cscArray->M;                    //Number of columns/row of the sparse matrix

    int colLength;      //number of nonzero elements of a column
    int rowNum;         //the row indice
    int colNum;         //the column indice
    int productNum;     //dot product of row with index rowNum with column with index colNum
    
    int* trianglesArray=calloc(M, sizeof(int)); //Array containing the number of triangles adjacent to each node i (M nodes in total)
    if(trianglesArray==NULL){
        printf("Error in main: Couldn't allocate memory for trianglesArray");
        exit(-1);
    }

    //Loop that calculates the number of triangles adjacent to each node
    //Check for each column
    for(int i=0; i<M; i++){ 
        colLength = colVector[i+1] - colVector[i];
        colNum=i;
        //Checking for each element of the column. This way I find every non zero element of the matrix.
        for(int j=0; j<colLength; j++){
            rowNum = rowVector[colVector[i]+j];
            //If this row contains only zeros, skip it. We take advantage of the fact that the row with index rowNum is the same with the column with index rowNum
            if(colVector[rowNum+1]-colVector[rowNum] == 0){
                continue;
            }
            productNum=product(rowVector, colVector, colNum, rowNum);
            trianglesArray[i] += productNum;
        }
        trianglesArray[i] /= 2;
    }

    int totalTriangles=0;

    //Calculate the total number of triangles
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