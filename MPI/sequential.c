#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifndef TOTAL_POINTS
#define TOTAL_POINTS 100000000
#endif

int main() {
    clock_t start_time = clock();
    
    long circle_count = 0;
    unsigned int seed = time(NULL);
    
    for (long i = 0; i < TOTAL_POINTS; i++) {
        double x = (double)rand_r(&seed) / RAND_MAX;
        double y = (double)rand_r(&seed) / RAND_MAX;
        
        double distance = x * x + y * y;
        
        if (distance <= 1.0) {
            circle_count++;
        }
    }
    
    double pi_estimate = 4.0 * (double)circle_count / TOTAL_POINTS;
    clock_t end_time = clock();
    double execution_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    
    printf("Sequential Version:\n");
    printf("Estimated Pi: %.10f\n", pi_estimate);
    printf("Execution Time: %.6f seconds\n", execution_time);
    printf("Total Points: %d\n", TOTAL_POINTS);
    printf("Points in Circle: %ld\n", circle_count);
    
    return 0;
}