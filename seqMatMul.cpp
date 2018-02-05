/*
 * File Name   :seqMatMul.cpp
 * Description :Implementation of sequential code for matrix multiplication
 * Author      :Karthik Rao 
 * Date        :Dec 09 2017
 * Version     :0.2

 * To compile :
 * make seqMatMul
 *
 * To compile :
 * g++ -std=c++11 seqMatMul.cpp -o seqMatMul
 * 
 * Sample command line execution :
 * 
 * g++ -n 9 ./seqMatMul 6
 *
 *
 * qsub -d $(pwd) -q mamba -l procs=2 -v mpirun -n 4 ./seqMatMul 8
 *
 */

/* Debug prints will be enabled if set to 1 */
#define DEBUG 0
#define NULL_MATRIX 0
#define IDENTITY_MATRIX 1
#define SPARSE_MATRIX 2
#define INCREMENTAL_VAL_ELEM 3
#define ALL_SET_1 4

#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <string.h>
#include <cmath>

#include "CommonHeader.h"


/* function to initialise the matrix A */
void InitMatrix (long long int *** matCur, int matRowSize, int matColmSize, int indexValue);
/* function to initialise the vector X */
void InitVector (long long int ** vectorCur, int matColmSize, int indexValue);
/* function to print the matrix of amy dimention */
void printMatrix(long long int **mat, int rows, int colms);
/* function to print the vector of amy dimention */
void printVector (long long int *vect, int rows);

/*==============================================================================
 *  main
 *=============================================================================*/

int main (int argc, char* argv[]) {


    if (argc < 2) {
        std::cerr<<"Usage: "<<argv[0]<<" <matSize>"<<std::endl;

        return -1;
    }



    int matSize  = atoi (argv[1]);

    int subMatColmSize, subMatRowSize;
    int firstRow, lastRow;
    int procRank = 0;

    long long int ** matrix;
    subMatColmSize = matSize;
    subMatRowSize = matSize;

    int i,j;
    /* measure time taken for integration */
    std::chrono::time_point<std::chrono::system_clock> StartTime;
    std::chrono::time_point<std::chrono::system_clock>  EndTime;
    std::chrono::duration<double> ElapsedTime;

    StartTime = std::chrono::system_clock::now();


    InitMatrix (&matrix, subMatRowSize, subMatColmSize, IDENTITY_MATRIX);

    firstRow = 0 ;
    lastRow  = subMatRowSize - 1;

    /* allocate memory for the received rows */
    long long int * vectorPast;
    long long int * vectorCur;
    InitVector (&vectorPast, subMatColmSize, INCREMENTAL_VAL_ELEM);
    InitVector (&vectorCur, subMatColmSize, NULL_MATRIX);


    DLOG (C_VERBOSE, "Node[%d] firstRow  = %d. lastRow  = %d\n",procRank, firstRow, lastRow);
    DLOG (C_VERBOSE, "Node[%d] subMatRowSize  = %d. subMatColmSize  = %d\n",procRank, subMatRowSize, subMatColmSize);

    /* end of first row subarray data type creation */
#if (DEBUG)
    DLOG (C_VERBOSE, "Node[%d] Printing matrix A\n",procRank);
    printMatrix (matrix, subMatRowSize, subMatColmSize);
#endif


    for (int k = 0; k < 20 ; k++) {

#if (DEBUG)
        DLOG (C_VERBOSE, "Node[%d] Printing vectorPast\n",procRank);
        printVector (vectorPast, subMatColmSize);
#endif

        memset (vectorCur, 0, subMatColmSize * sizeof(long long int));

        for (i = 0; i < subMatRowSize; i++) {

            for (j = 0; j < subMatColmSize; j++) {

                vectorCur[i] = vectorCur[i] + vectorPast[j] * matrix[i][j];   
            }
        }


#if (DEBUG)
        DLOG (C_VERBOSE, "Node[%d] Printing vectorCur\n",procRank);
        printVector (vectorCur, subMatRowSize);
#endif


        DLOG (C_VERBOSE, "Node[%d] Copying matCur to matrix\n",procRank);
        for (i = 0; i < subMatRowSize ; i++){
            memcpy ( &vectorPast[0], &vectorCur[0], subMatColmSize * sizeof(long long int));
        }


    }/* end of loop to compute 20 iterations */

    /* compute the time taken to compute the sum and display the same */
    EndTime = std::chrono::system_clock::now();
    ElapsedTime = EndTime - StartTime;

    std::cerr<<ElapsedTime.count()<<std::endl;

    delete [] vectorCur;
    delete [] vectorPast;

    for (i = 0; i < subMatRowSize; i++){
        delete [] matrix[i];
    }
    delete [] matrix;




    return 0;
}


/*==============================================================================
 *  InitMatrix
 *=============================================================================*/

void InitMatrix (long long int *** matCur, int matRowSize, int matColmSize, int indexValue) {

    int i,j;
    long long int ** matrix = new long long int * [matRowSize];

    (*matCur) = matrix;
    int procRank = 0;

    DLOG (C_VERBOSE, "Node[%d] Enter\n",procRank);

    for (i = 0; i < matRowSize; i++){
        matrix[i] = new long long int [matColmSize];
        (*matCur)[i] = matrix[i];
    }

    if (indexValue == 0) {
        /* set the array elements to 0, use memset */

        for (i = 0; i < matRowSize ; i++ ) {
            for ( j = 0; j < matColmSize ; j++ ) {
                matrix[i][j] = 0; 
            }
        }

    } else if (indexValue == IDENTITY_MATRIX) {
        /* identity matix */

        for (i = 0; i < matRowSize ; i++ ) {
            for ( j = 0; j < matColmSize ; j++ ) {


                if ( i == j) {
                    matrix[i][j] = 1; 
                } else {
                    matrix[i][j] = 0; 
                }


            }
        }

    } else if (indexValue == SPARSE_MATRIX) {
        /* sparse matrix  */
        for (i = 0; i < matRowSize ; i++ ) {
            for ( j = 0; j < matColmSize ; j++ ) {


                if ( (i+j) % 2 == 0) {
                    matrix[i][j] = 1; 
                } else {
                    matrix[i][j] = 0; 
                }
            }
        }


    }


    DLOG (C_VERBOSE, "Node[%d] Exit\n",procRank);
}


/*==============================================================================
 *  InitVector
 *=============================================================================*/

void InitVector (long long int ** vectorCur, int matColmSize, int indexValue) {

    int i,j;
    long long int * vector = new long long int [matColmSize];

    (*vectorCur) = vector;
    int procRank = 0;

    DLOG (C_VERBOSE, "Node[%d] Enter\n",procRank);

    if (indexValue == NULL_MATRIX) {

        memset (vector, 0, matColmSize * sizeof(long long int));

    } else if (indexValue == INCREMENTAL_VAL_ELEM) {
        /* set elements in a particular order */

        for (i = 0; i < matColmSize ; i++ ) {
            vector[i] = i; 


        }

    } else if (indexValue == ALL_SET_1) {
        
        for (i = 0; i < matColmSize ; i++ ) {

            vector[i] = 1; 

        }

    }


    DLOG (C_VERBOSE, "Node[%d] Exit\n",procRank);
}


/*==============================================================================
 *  printMatrix
 *=============================================================================*/

void printMatrix (long long int **mat, int rows, int colms)
{
    int i, j;

    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < colms; j++)
        {
            std::cout<<mat[i][j]<<" ";
        }
        std::cout<<std::endl;
    }
}

/*==============================================================================
 *  printVector
 *=============================================================================*/

void printVector (long long int *vect, int rows)
{
    int i, j;

    for (i = 0; i < rows; i++)
    {
        std::cout<<vect[i]<<" ";
    }
    std::cout<<std::endl;

}

