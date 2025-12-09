/*
 * matrixOp_server.c - Server implementation for matrix operation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "matrixOp.h"

#define EPSILON 1e-10

/* Helper function to create a matrix */
static matrix *create_matrix(int rows, int cols) {
    matrix *mat = (matrix *)malloc(sizeof(matrix));
    if (!mat) return NULL;
    
    mat->rows = rows;
    mat->cols = cols;
    mat->data.data_len = rows * cols;
    mat->data.data_val = (double *)calloc(rows * cols, sizeof(double)); // Use calloc to initialize to 0
    
    return mat;
}

/* Get element from matrix */
static double get_element(const matrix *mat, int i, int j) {
    if (i < mat->rows && j < mat->cols) {
        return mat->data.data_val[i * mat->cols + j];
    }
    return 0.0;
}

/* Set element in matrix */
static void set_element(matrix *mat, int i, int j, double value) {
    if (i < mat->rows && j < mat->cols) {
        mat->data.data_val[i * mat->cols + j] = value;
    }
}

/* Matrix addition: C = A + B */
matrix_result *matrix_add_1_svc(matrix_pair *pair, struct svc_req *req) {
    static matrix_result result;
    matrix *a = &pair->first;
    matrix *b = &pair->second;
    
    /* Initialize result */
    memset(&result, 0, sizeof(result));
    result.success = 0;
    result.error_msg = "";
    
    /* Check if matrices have same dimensions */
    if (a->rows != b->rows || a->cols != b->cols) {
        result.error_msg = "Error: Matrices must have same dimensions for addition";
        return &result;
    }
    
    /* Create result matrix */
    matrix *result_mat = create_matrix(a->rows, a->cols);
    if (!result_mat) {
        result.error_msg = "Error: Memory allocation failed";
        return &result;
    }
    
    /* Perform addition */
    for (int i = 0; i < a->rows; i++) {
        for (int j = 0; j < a->cols; j++) {
            double sum = get_element(a, i, j) + get_element(b, i, j);
            set_element(result_mat, i, j, sum);
        }
    }
    
    result.success = 1;
    result.result_matrix = *result_mat; // Copy the structure
    
    return &result;
}

/* Matrix multiplication: C = A * B */
matrix_result *matrix_mult_1_svc(matrix_pair *pair, struct svc_req *req) {
    static matrix_result result;
    matrix *a = &pair->first;
    matrix *b = &pair->second;
    
    /* Initialize result */
    memset(&result, 0, sizeof(result));
    result.success = 0;
    result.error_msg = "";
    
    /* Check if matrices can be multiplied */
    if (a->cols != b->rows) {
        result.error_msg = "Error: Incompatible dimensions for multiplication";
        return &result;
    }
    
    /* Create result matrix */
    matrix *result_mat = create_matrix(a->rows, b->cols);
    if (!result_mat) {
        result.error_msg = "Error: Memory allocation failed";
        return &result;
    }
    
    /* Perform multiplication */
    for (int i = 0; i < a->rows; i++) {
        for (int j = 0; j < b->cols; j++) {
            double sum = 0.0;
            for (int k = 0; k < a->cols; k++) {
                sum += get_element(a, i, k) * get_element(b, k, j);
            }
            set_element(result_mat, i, j, sum);
        }
    }
    
    result.success = 1;
    result.result_matrix = *result_mat;
    
    return &result;
}

/* Matrix transpose: B = A^T */
matrix_result *matrix_transpose_1_svc(matrix *a, struct svc_req *req) {
    static matrix_result result;
    
    /* Initialize result */
    memset(&result, 0, sizeof(result));
    result.success = 0;
    result.error_msg = "";
    
    /* Create result matrix */
    matrix *result_mat = create_matrix(a->cols, a->rows);
    if (!result_mat) {
        result.error_msg = "Error: Memory allocation failed";
        return &result;
    }
    
    /* Perform transpose */
    for (int i = 0; i < a->rows; i++) {
        for (int j = 0; j < a->cols; j++) {
            set_element(result_mat, j, i, get_element(a, i, j));
        }
    }
    
    result.success = 1;
    result.result_matrix = *result_mat;
    
    return &result;
}

/* Better matrix inverse using LU decomposition */
static int matrix_inverse_lu(double *A, double *inv, int n) {
    int i, j, k;
    
    // Initialize inverse as identity matrix
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            inv[i * n + j] = (i == j) ? 1.0 : 0.0;
        }
    }
    
    // LU decomposition with partial pivoting
    for (k = 0; k < n; k++) {
        // Find pivot
        int pivot = k;
        double max_val = fabs(A[k * n + k]);
        for (i = k + 1; i < n; i++) {
            if (fabs(A[i * n + k]) > max_val) {
                max_val = fabs(A[i * n + k]);
                pivot = i;
            }
        }
        
        if (max_val < EPSILON) {
            return 0; // Matrix is singular
        }
        
        // Swap rows if needed
        if (pivot != k) {
            for (j = 0; j < n; j++) {
                double temp = A[k * n + j];
                A[k * n + j] = A[pivot * n + j];
                A[pivot * n + j] = temp;
                
                temp = inv[k * n + j];
                inv[k * n + j] = inv[pivot * n + j];
                inv[pivot * n + j] = temp;
            }
        }
        
        // Eliminate below
        for (i = k + 1; i < n; i++) {
            double factor = A[i * n + k] / A[k * n + k];
            for (j = k; j < n; j++) {
                A[i * n + j] -= factor * A[k * n + j];
            }
            for (j = 0; j < n; j++) {
                inv[i * n + j] -= factor * inv[k * n + j];
            }
        }
    }
    
    // Back substitution
    for (k = n - 1; k >= 0; k--) {
        for (j = 0; j < n; j++) {
            inv[k * n + j] /= A[k * n + k];
        }
        
        for (i = 0; i < k; i++) {
            for (j = 0; j < n; j++) {
                inv[i * n + j] -= A[i * n + k] * inv[k * n + j];
            }
        }
    }
    
    return 1;
}

/* Matrix inverse: B = A^(-1) */
matrix_result *matrix_inverse_1_svc(matrix *a, struct svc_req *req) {
    static matrix_result result;
    
    /* Initialize result */
    memset(&result, 0, sizeof(result));
    result.success = 0;
    result.error_msg = "";
    
    /* Check if matrix is square */
    if (a->rows != a->cols) {
        result.error_msg = "Error: Only square matrices can be inverted";
        return &result;
    }
    
    int n = a->rows;
    
    /* Create result matrix */
    matrix *result_mat = create_matrix(n, n);
    if (!result_mat) {
        result.error_msg = "Error: Memory allocation failed";
        return &result;
    }
    
    /* Create working copy of the matrix */
    double *A_copy = (double *)malloc(n * n * sizeof(double));
    if (!A_copy) {
        result.error_msg = "Error: Memory allocation failed";
        return &result;
    }
    
    /* Copy matrix data */
    for (int i = 0; i < n * n; i++) {
        A_copy[i] = a->data.data_val[i];
    }
    
    /* Perform matrix inversion */
    if (matrix_inverse_lu(A_copy, result_mat->data.data_val, n)) {
        result.success = 1;
        result.result_matrix = *result_mat;
    } else {
        result.error_msg = "Error: Matrix is singular and cannot be inverted";
    }
    
    free(A_copy);
    
    return &result;
}

/* Test connection */
int *ping_1_svc(void *argp, struct svc_req *req) {
    static int result = 1;
    return &result;
}