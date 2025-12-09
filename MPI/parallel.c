#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>
#include <time.h>

#ifndef TOTAL_POINTS
#define TOTAL_POINTS 100000000
#endif

int main(int argc, char *argv[]) {
    int rank, size;
    long points_per_process;
    long local_circle_count = 0;
    long total_circle_count = 0;
    double start_time, end_time;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    points_per_process = TOTAL_POINTS / size;
    
    // Master process
    if (rank == 0) {
        start_time = MPI_Wtime();
        printf("Starting parallel calculation with %d processes\n", size);
        printf("Points per process: %ld\n", points_per_process);
    }
    
    // Each process calculates its portion
    unsigned int seed = time(NULL) + rank;
    for (long i = 0; i < points_per_process; i++) {
        double x = (double)rand_r(&seed) / RAND_MAX;
        double y = (double)rand_r(&seed) / RAND_MAX;
        
        if (x * x + y * y <= 1.0) {
            local_circle_count++;
        }
    }
    
    // Reduce all local counts to master
    MPI_Reduce(&local_circle_count, &total_circle_count, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    
    // Master calculates and displays results
    if (rank == 0) {
        double pi_estimate = 4.0 * (double)total_circle_count / TOTAL_POINTS;
        end_time = MPI_Wtime();
        
        printf("\nParallel Version Results:\n");
        printf("Estimated Pi: %.10f\n", pi_estimate);
        printf("Execution Time: %.6f seconds\n", end_time - start_time);
        printf("Total Points: %d\n", TOTAL_POINTS);
        printf("Points in Circle: %ld\n", total_circle_count);
        printf("Number of Processes: %d\n", size);
    }
    
    MPI_Finalize();
    return 0;
}