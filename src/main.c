#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

//Variables globales

void master() {

    

}


int main (int argc, char* argv[]) {
    
    int rank;
    int procs_number;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &procs_number);

    if (rank == 0) {
        //Master

        read_wav()

    } else {
        //Workers 


    }



}