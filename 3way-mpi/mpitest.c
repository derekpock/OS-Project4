#include <stdio.h>
#include <malloc.h>
#include "mpi.h"

int main(int argc, char *argv[])
{
    int threadId, numOfThreads;

    /* Start up MPI */

    int *yValue;
    yValue = malloc(sizeof(int));
    *yValue = 1;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &threadId);
    MPI_Comm_size(MPI_COMM_WORLD, &numOfThreads);

    int *xValue;
    xValue = malloc(sizeof(int));
    *xValue = 1;

    if (threadId == 0) {
        *xValue = 2;
        *yValue = 2;
//        printf("Enter the number of times around the ring: ");
//        scanf("%d", &num);
//        --num;
//
//        printf("Process %d sending %d to %d\n", rank, num, next);
//        MPI_Send(&num, 1, MPI_INT, next, tag, MPI_COMM_WORLD);
    }

    /* Pass the message around the ring.  The exit mechanism works */
    /* as follows: the message (a positive integer) is passed */
    /* around the ring.  Each time is passes rank 0, it is decremented. */
    /* When each processes receives the 0 message, it passes it on */
    /* to the next process and then quits.  By passing the 0 first, */
    /* every process gets the 0 message and can quit normally. */

//    while (1) {
//
//        MPI_Recv(&num, 1, MPI_INT, from, tag, MPI_COMM_WORLD, &status);
//        printf("Process %d received %d\n", rank, num);
//
//        if (rank == 0) {
//            num--;
//            printf("Process 0 decremented num\n");
//        }
//
//        printf("Process %d sending %d to %d\n", rank, num, next);
//        MPI_Send(&num, 1, MPI_INT, next, tag, MPI_COMM_WORLD);
//
//        if (num == 0) {
//            printf("Process %d exiting\n", rank);
//            break;
//        }
//    }

    /* The last process does one extra send to process 0, which needs */
    /* to be received before the program can exit */

//    if (rank == 0)
//        MPI_Recv(&num, 1, MPI_INT, from, tag, MPI_COMM_WORLD, &status);

    /* Quit */

    printf("Thread %d has values of before %d and after %d.\n", threadId, *yValue, *xValue);

    MPI_Finalize();
    return 0;
}
