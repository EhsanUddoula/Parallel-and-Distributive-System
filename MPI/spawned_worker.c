#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>

int main(int argc, char *argv[]) {
    int points_per_worker;
    long local_circle_count = 0;
    MPI_Status status;
    MPI_Comm parent;
    
    MPI_Init(&argc, &argv);
    
    // Get the parent communicator
    MPI_Comm_get_parent(&parent);
    
    if (parent == MPI_COMM_NULL) {
        printf("Worker Error: Not spawned by a parent!\n");
        MPI_Finalize();
        return 1;
    }
    
    // Receive number of points from master
    MPI_Recv(&points_per_worker, 1, MPI_INT, 0, 0, parent, &status);
    
    //printf("Worker: Received %d points to process\n", points_per_worker);
    
    unsigned int seed = time(NULL);
    for (int i = 0; i < points_per_worker; i++) {
        double x = (double)rand_r(&seed) / RAND_MAX;
        double y = (double)rand_r(&seed) / RAND_MAX;
        
        if (x * x + y * y <= 1.0) {
            local_circle_count++;
        }
    }
    
    //printf("Worker: Sending result %ld back to master\n", local_circle_count);
    
    // Send result back to master
    MPI_Send(&local_circle_count, 1, MPI_LONG, 0, 0, parent);
    
    MPI_Finalize();
    return 0;
}