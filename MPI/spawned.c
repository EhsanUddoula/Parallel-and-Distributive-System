#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <math.h>
#include <time.h>

#ifndef TOTAL_POINTS
#define TOTAL_POINTS 100000000
#endif

int main(int argc, char *argv[]) {
    int world_rank;
    MPI_Comm worker_comm;
    int num_workers;
    int points_per_worker;
    long total_circle_count = 0;
    double start_time, end_time;
    int errcodes[8];
    char worker_path[256] = "./bin/spawned_worker";  // Path to worker executable
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    
    if (argc < 2) {
        if (world_rank == 0) {
            printf("Usage: %s <num_workers>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }
    
    num_workers = atoi(argv[1]);
    points_per_worker = TOTAL_POINTS / num_workers;
    
    if (world_rank == 0) {
        printf("Master: Dynamically spawning %d workers\n", num_workers);
        printf("Master: Points per worker: %d\n", points_per_worker);
        printf("Master: Worker path: %s\n", worker_path);
        start_time = MPI_Wtime();
        
        // Spawn worker processes with full path
        MPI_Comm_spawn(worker_path, MPI_ARGV_NULL, num_workers,
                      MPI_INFO_NULL, 0, MPI_COMM_SELF, &worker_comm, errcodes);
        
        printf("Master: Workers spawned successfully\n");
        
        // Send work to all workers
        for (int i = 0; i < num_workers; i++) {
            MPI_Send(&points_per_worker, 1, MPI_INT, i, 0, worker_comm);
        }
        printf("Master: Sent work to all workers\n");
        
        // Receive results from workers
        for (int i = 0; i < num_workers; i++) {
            long worker_count;
            MPI_Status status;
            MPI_Recv(&worker_count, 1, MPI_LONG, i, 0, worker_comm, &status);
            total_circle_count += worker_count;
            printf("Master: Received result from worker %d: %ld points\n", i, worker_count);
        }
        
        double pi_estimate = 4.0 * (double)total_circle_count / TOTAL_POINTS;
        end_time = MPI_Wtime();
        
        printf("\nDynamic Spawning Results:\n");
        printf("Estimated Pi: %.10f\n", pi_estimate);
        printf("Execution Time: %.6f seconds\n", end_time - start_time);
        printf("Total Points: %d\n", TOTAL_POINTS);
        printf("Points in Circle: %ld\n", total_circle_count);
        printf("Number of Workers: %d\n", num_workers);
    }
    
    MPI_Finalize();
    return 0;
}