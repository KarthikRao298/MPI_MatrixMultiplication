/*
 * File Name   :matMul.cpp
 * Description :Implementation of MPI based parallel code for matrix multiplication
 *               with block decomposition based partitioning
 * Author      :Karthik Rao 
 * Date        :Dec 09 2017
 * Version     :1.1
 * 
 * To compile :
 * make matMul
 *
 * To compile :
 * mpicxx -std=c++11 matMul.cpp -o matMul
 * 
 * Sample command line execution :
 * 
 * mpirun -n 4 ./matMul 4
 *
 */

/* Debug prints will be enabled if set to 1 */
#define DEBUG 0
#define NODE_0 0
#define TWO_DIMENSION 2
#define NO_REORDER 0

#define NULL_MATRIX 0
#define IDENTITY_MATRIX 1
#define SPARSE_MATRIX 2
#define INCREMENTAL_VAL_ELEM 3
#define ALL_SET_1 4

#define VECTOR_COLUMN_RESULT 12

#include <mpi.h>
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


/* function allocates memory & initializes the matrix A */
void InitMatrix (long long int *** matCur, int matRowSize, int matColmSize, int indexValue);
/* function allocates memory & initializes the vector X */
void InitVector (long long int ** vectorCur, int matColmSize, int indexValue);
/* function to print the matrix of amy dimention */
void printMatrix(long long int **mat, int rows, int colms);
/* function to print the vector of amy dimention */
void printVector (long long int *vect, int rows);



/*==============================================================================
 *  main
 *=============================================================================*/

int main (int argc, char* argv[]) {



    /* 
     * 1.  at every node create a block of submatrix of matrix A, i.e matrix A is distributed across all the nodes
     * 2.  create 2D cartesin topology of nodes resulting in a grid. group all the nodes in a row as a single communicator.
     *     Similarly group all the nodes in the column as one communicator
     * 2.1 generate the few columns of the vector X(t-1) at nodes with coordinates (0,i) in the grid. The nodes with coordinate (0,i) are 
     *     the leaders for the row communicator similarly (i,0) nodes are the leaders for the column communicators.
     * 3.  at every leader node, broadcast the columns of X(t-1) to its column communicator group
     * 4.  every node multiplies the sub matrix A with the corresponding columns of vector X(t-1)
     * 5.  The result of the multiplication at every row communicators gets reduced at the leader.
     *     Hence, the leader of each row communicator will have the few columns of the resultant X(t).
     * 5.1 The row leaders sends the result X(t) to the column leaders.    
     * 6.  This X(t) would be used as X(t-1) for the next computation. The column leaders would braodcast the X(t) to all the
     *     nodes in its column communicator. If 20 iterations have not been done then go to step 3, else goto step 7
     * 7.  gather the columns of X(t) from every column leader at node 0 in the world communicator.
     *
     */ 

    MPI_Init(NULL, NULL);

    int matSize  = atoi (argv[1]);

    int numprocs, myWorldRank;
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myWorldRank);

    if (argc < 2) {
        std::cerr<<"Usage: "<<argv[0]<<" <MatrixSize>"<<std::endl;
        MPI_Finalize();
        return -1;
    }

    if ( matSize < 4) {
        DLOG (C_ERROR, " matrix size should be greater than 4\n");
        MPI_Finalize();
        return -1;
    }

    if ( ((matSize * matSize) % numprocs) !=0) {
        DLOG (C_ERROR, " matrix size not compatible with the no of processors\n");
        MPI_Finalize();
        return -1;
    }

    std::chrono::time_point<std::chrono::system_clock> StartTime;
    std::chrono::time_point<std::chrono::system_clock>  EndTime;
    std::chrono::duration<double> ElapsedTime;

    MPI_Barrier( MPI_COMM_WORLD ) ;
    if ( myWorldRank == NODE_0){
        StartTime = std::chrono::system_clock::now();
    }
    MPI_Barrier( MPI_COMM_WORLD ) ;


    int i, j;
    int dest_rank;
    int size[2], coords[2];
    MPI_Comm grid_comm;



    long long int ** matrix;
    int subMatColmSize = matSize / sqrt(numprocs);
    int subMatRowSize = matSize / sqrt(numprocs);
    InitMatrix (&matrix, subMatRowSize, subMatColmSize, IDENTITY_MATRIX);

    long long int * vectorPast;
    long long int * vectorFinalResult = NULL;
    long long int * vectorCur;
    long long int * vectorResult;
    int subVecColmSize = subMatColmSize;
    InitVector (&vectorCur, subVecColmSize, NULL_MATRIX);
    InitVector (&vectorResult, subVecColmSize, NULL_MATRIX);

    if ( myWorldRank == NODE_0) {
        DLOG (C_VERBOSE, "Node[%d] initializing vector to store the result\n", myWorldRank);

        InitVector (&vectorFinalResult, matSize, NULL_MATRIX);
    }

#if (DEBUG)
    DLOG (C_VERBOSE, "Node[%d] Printing matrix A\n", myWorldRank);
    printMatrix (matrix, subMatRowSize, subMatColmSize);

#endif

    /*
     * create a contigious vector type
     */
    DLOG (C_VERBOSE, "Node[%d] creating user defined vector data type\n", myWorldRank);
    MPI_Datatype vectType ;
    MPI_Type_contiguous (subVecColmSize , MPI_LONG_LONG_INT , &vectType );
    MPI_Type_commit (&vectType );

    DLOG (C_VERBOSE, "Node[%d] creating cartesian grid\n", myWorldRank);
    size[0] = sqrt(numprocs);
    size[1] = sqrt(numprocs); /* this should be sqrt(noOfProc)*/
    int periodic[2] = {0,0};/* no wrapping around */
    int grid_coords[2] = {0,0};

    /* 
     * Create virtual grid
    * arg1 - old Communicator from which the new communicator is created
     * arg2 - Number of dimensions of the cartesian topology,
     * arg3 - grid size in each dimension
     * arg4 - periodicity in each dimension;this is used to deal with elements on the boundaries
     * arg5 - 1=reorder processes, 0=do not reorder processes
     * arg6 - the new cartesian grid communicator
     * */
    MPI_Cart_create (MPI_COMM_WORLD, TWO_DIMENSION, size, periodic, NO_REORDER, &grid_comm);
    /* this function gives the coordinates of the node in the 2D grid */
    MPI_Cart_coords (grid_comm, myWorldRank, TWO_DIMENSION, grid_coords);



    DLOG (C_VERBOSE, "Node[%d] creating row & column communicators\n", myWorldRank);
    MPI_Comm comm_row;
    MPI_Comm comm_colm;


    MPI_Comm_split (grid_comm, grid_coords[1], grid_coords[0], &comm_colm); 
    MPI_Comm_split (grid_comm, grid_coords[0], grid_coords[1], &comm_row);


    int rowRank, colmRank;
    MPI_Comm_rank(comm_row, &rowRank);
    MPI_Comm_rank(comm_colm, &colmRank);

    DLOG (C_VERBOSE, "Node[%d] myWorldRank = %d. grid_coords[0] = %d grid_coords[1] = %d "
            "rowRank = %d colmRank = %d\n", myWorldRank, myWorldRank, grid_coords[0], grid_coords[1], rowRank, colmRank );

    /*
     * initialize vector at the first row nodes and broadcast it
     */
    if ( grid_coords[0] == 0) {
        DLOG (C_VERBOSE, "Node[%d] is a leader! initializing vector\n", myWorldRank);
        InitVector (&vectorPast, subVecColmSize, INCREMENTAL_VAL_ELEM);
#if (DEBUG)
        DLOG (C_VERBOSE, "Node[%d] Printing vectorPast\n", myWorldRank);
        printVector (vectorPast, subVecColmSize);
#endif
    
    } else {
        DLOG (C_VERBOSE, "Node[%d] Not a leader, so initilizing vector with 0\n", myWorldRank);
        InitVector (&vectorPast, subVecColmSize, NULL_MATRIX);
    }

    DLOG (C_VERBOSE, "Node[%d] broadcasting the initial vector\n", myWorldRank);
    MPI_Bcast (vectorPast, 1, vectType, NODE_0, comm_colm);

#if (DEBUG)
    DLOG (C_VERBOSE, "Node[%d] Printing vectorPast\n", myWorldRank);
    printVector (vectorPast, subVecColmSize);
#endif

    for (int k = 0; k < 20; k++) {

   
        DLOG (C_VERBOSE, "Node[%d] clearing the vectorCur\n", myWorldRank);
        memset (vectorCur, 0, subMatColmSize * sizeof(long long int));

        DLOG (C_VERBOSE, "Node[%d] computing matrix-vector multiplication\n", myWorldRank);
        for (i = 0; i < subMatRowSize; i++) {
            for (j = 0; j < subMatColmSize; j++) {

                vectorCur[i] = vectorCur[i] + vectorPast[j] * matrix[i][j];   
            }
        }
#if (DEBUG)
        DLOG (C_VERBOSE, "Node[%d] Printing vectorCur\n", myWorldRank);
        printVector (vectorCur, subVecColmSize);
#endif

/*
 * reduce the multiplication result at leader node of row communicators 
 */

        DLOG (C_VERBOSE, "Node[%d] Reducing the vector result at NODE_0 of row communicators\n", myWorldRank);
        MPI_Reduce(vectorCur, vectorResult, subVecColmSize, MPI_LONG_LONG_INT, MPI_SUM, NODE_0, comm_row);

#if (DEBUG)
        DLOG (C_VERBOSE, "Node[%d] Printing vectorResult\n", myWorldRank);
        printVector (vectorResult, subVecColmSize);
#endif


        /*
         * row leaders send result to the column leaders
         */
        if (grid_coords[0] == 0 && grid_coords[1] == 0) {/* (0,0) node so do nothing */  

            DLOG (C_VERBOSE, "Node[%d] Not sending result to any nodes\n", myWorldRank);
        } else if (grid_coords[1] == 0) {/* if 1st column node then send the result to corrsponding 1st row node */

            coords[1] = grid_coords[0];/* assuming grid_coords[0] is the row coordinate */
            coords[0] = 0;
            MPI_Cart_rank (grid_comm, coords, &dest_rank);

            DLOG (C_VERBOSE, "Node[%d] sending result to node[%d]\n", myWorldRank, dest_rank);
            MPI_Send (vectorResult, 1, vectType, dest_rank, VECTOR_COLUMN_RESULT, grid_comm);
        } else if (grid_coords[0] == 0) {/* if 1st row node then receive the result from the corrsponding 1st column node */

            coords[0] = grid_coords[1];
            coords[1] = 0;
            MPI_Cart_rank (grid_comm, coords, &dest_rank);

            DLOG (C_VERBOSE, "Node[%d] receiving result from node[%d]\n", myWorldRank, dest_rank);
            MPI_Recv (vectorResult, 1, vectType, dest_rank, VECTOR_COLUMN_RESULT, grid_comm, MPI_STATUS_IGNORE );
        }



        DLOG (C_VERBOSE, "Node[%d] broadcasting the result to column communicators\n", myWorldRank);

        MPI_Bcast (vectorResult, 1, vectType, NODE_0, comm_colm);

        DLOG (C_VERBOSE, "Node[%d] copying vectorResult to vectorPast \n", myWorldRank);

        memcpy ( vectorPast, vectorResult, subVecColmSize * sizeof(long long int));

#if (DEBUG)
        if (grid_coords[1] == 0 ) {
            DLOG (C_VERBOSE, "Node[%d] Printing vectorResult \n", myWorldRank);
            printVector (vectorResult, subVecColmSize);
        }
#endif
        DLOG (C_VERBOSE, "Node[%d] Barrier encountered! \n", myWorldRank);

        /* to debug send the reduced vector from all the first colums to the NODE_0 in MPI_COMM_WORLD */
        if (grid_coords[1] == 0)
        {

            DLOG (C_VERBOSE, "Node[%d] gathering result at node 0\n", myWorldRank);
            MPI_Gather (vectorResult, 1, vectType, vectorFinalResult, 1, vectType, NODE_0, comm_colm);
        }

#if (DEBUG)
        if (myWorldRank == NODE_0) {
            DLOG (C_VERBOSE, "Node[%d] Printing vectorFinalResult of the multiplication at iteration = %d\n", myWorldRank, k);
            printVector (vectorFinalResult, matSize);
        }
#endif
        DLOG (C_VERBOSE, "Node[%d] Barrier encountered! \n", myWorldRank);
        MPI_Barrier( MPI_COMM_WORLD ) ;/* might not be needed */

    }/* end of for loop */


    MPI_Barrier( MPI_COMM_WORLD ) ;
    /* compute the time taken for the computation */
    if ( myWorldRank == NODE_0){
        EndTime = std::chrono::system_clock::now();
        ElapsedTime = EndTime - StartTime;

        std::cerr<<ElapsedTime.count()<<std::endl;
    }
    MPI_Barrier( MPI_COMM_WORLD ) ;



    /* gather the result at node 0 of world communicator */
    if (myWorldRank == NODE_0) {
        delete [] vectorFinalResult;

    }
    delete [] vectorCur;
    delete [] vectorPast;
    delete [] vectorResult;

    for (i = 0; i < subMatRowSize; i++){
        delete [] matrix[i];
    }
    delete [] matrix;

    MPI_Comm_free (&grid_comm);
    MPI_Comm_free (&comm_row);
    MPI_Comm_free (&comm_colm);

    MPI_Type_free (&vectType);


    MPI_Finalize();
    return 0;

}

/*==============================================================================
 *  InitMatrix
 *=============================================================================*/

void InitMatrix (long long int *** matCur, int matRowSize, int matColmSize, int indexValue) {

    int i,j;
    long long int ** matrix = new long long int * [matRowSize];

    (*matCur) = matrix;
    int  myWorldRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myWorldRank);

    DLOG (C_VERBOSE, "Node[%d] Enter\n", myWorldRank);

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


    DLOG (C_VERBOSE, "Node[%d] Exit\n", myWorldRank);
}


/*==============================================================================
 *  InitVector
 *=============================================================================*/

void InitVector (long long int ** vectorCur, int matColmSize, int indexValue) {

    int i;
    long long int * vector = new long long int [matColmSize];

    (*vectorCur) = vector;
    int  myWorldRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myWorldRank);

    DLOG (C_VERBOSE, "Node[%d] Enter\n", myWorldRank);

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


    DLOG (C_VERBOSE, "Node[%d] Exit\n", myWorldRank);
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
    int i;

    for (i = 0; i < rows; i++)
    {
        std::cout<<vect[i]<<" ";
    }
    std::cout<<std::endl;

}

