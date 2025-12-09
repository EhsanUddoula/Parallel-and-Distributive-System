/*
 * matrixOp_client.c - Client implementation with both test and interactive modes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "matrixOp.h"

/* Function to print a matrix */
void print_matrix(const matrix *mat) {
    printf("Matrix (%dx%d):\n", mat->rows, mat->cols);
    for (int i = 0; i < mat->rows; i++) {
        printf("  ");
        for (int j = 0; j < mat->cols; j++) {
            printf("%8.3f ", mat->data.data_val[i * mat->cols + j]);
        }
        printf("\n");
    }
}

/* Function to create a matrix from user input */
matrix *input_matrix(const char *name) {
    int rows, cols;
    
    printf("Enter dimensions for matrix %s (rows cols): ", name);
    if (scanf("%d %d", &rows, &cols) != 2) {
        printf("Invalid input!\n");
        return NULL;
    }
    
    if (rows <= 0 || cols <= 0 || rows * cols > MAX_SIZE) {
        printf("Invalid dimensions! Maximum size is %d elements.\n", MAX_SIZE);
        return NULL;
    }
    
    // Allocate matrix dynamically
    matrix *mat = (matrix *)malloc(sizeof(matrix));
    if (!mat) {
        printf("Memory allocation failed for matrix structure!\n");
        return NULL;
    }
    
    mat->rows = rows;
    mat->cols = cols;
    mat->data.data_len = rows * cols;
    mat->data.data_val = (double *)malloc(rows * cols * sizeof(double));
    
    if (!mat->data.data_val) {
        printf("Memory allocation failed for matrix data!\n");
        free(mat);
        return NULL;
    }
    
    printf("Enter %d elements for matrix %s (row by row):\n", rows * cols, name);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("  [%d][%d]: ", i, j);
            if (scanf("%lf", &mat->data.data_val[i * cols + j]) != 1) {
                printf("Invalid input!\n");
                free(mat->data.data_val);
                free(mat);
                return NULL;
            }
        }
    }
    
    return mat;
}

/* Function to create a predefined matrix for testing */
matrix *create_test_matrix(int rows, int cols, double *data) {
    // Allocate matrix dynamically
    matrix *mat = (matrix *)malloc(sizeof(matrix));
    if (!mat) return NULL;
    
    mat->rows = rows;
    mat->cols = cols;
    mat->data.data_len = rows * cols;
    mat->data.data_val = (double *)malloc(rows * cols * sizeof(double));
    
    if (!mat->data.data_val) {
        free(mat);
        return NULL;
    }
    
    if (data) {
        memcpy(mat->data.data_val, data, rows * cols * sizeof(double));
    }
    
    return mat;
}

/* Function to create a matrix pair */
matrix_pair create_matrix_pair(matrix *first, matrix *second) {
    matrix_pair pair;
    pair.first = *first;
    pair.second = *second;
    return pair;
}

void run_client_test(const char *server_address, int client_id) {
    CLIENT *clnt;
    matrix_result *result;
    int *ping_result;
    
    printf("\n=== Client %d Connecting to %s ===\n", client_id, server_address);
    
    /* Create client handle */
    clnt = clnt_create(server_address, MATRIX_OPERATIONS_PROG, MATRIX_OPERATIONS_VERS, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(server_address);
        printf("Client %d: Failed to connect to server at %s\n", client_id, server_address);
        return;
    }
    
    /* Test connection */
    ping_result = ping_1(NULL, clnt);
    if (ping_result == NULL) {
        clnt_perror(clnt, "call failed");
        printf("Client %d: Server at %s not responding\n", client_id, server_address);
        clnt_destroy(clnt);
        return;
    }
    
    printf("Client %d: Connected to server at %s successfully\n", client_id, server_address);
    
    /* Test matrices */
    double dataA[] = {1, 2, 3, 4};
    double dataB[] = {5, 6, 7, 8};
    double dataC[] = {4, 7, 2, 6};  // Invertible 2x2 matrix
    
    matrix *A = create_test_matrix(2, 2, dataA);
    matrix *B = create_test_matrix(2, 2, dataB);
    matrix *C = create_test_matrix(2, 2, dataC);
    
    if (!A || !B || !C) {
        printf("Client %d: Failed to create test matrices\n", client_id);
        clnt_destroy(clnt);
        return;
    }
    
    /* Test matrix operations */
    printf("\nClient %d: Testing Matrix Addition\n", client_id);
    matrix_pair add_pair = create_matrix_pair(A, B);
    result = matrix_add_1(&add_pair, clnt);
    if (result == NULL) {
        clnt_perror(clnt, "call failed");
    } else if (result->success) {
        printf("Addition successful!\n");
        print_matrix(&result->result_matrix);
    } else {
        printf("Addition failed: %s\n", result->error_msg);
    }
    
    printf("\nClient %d: Testing Matrix Multiplication\n", client_id);
    matrix_pair mult_pair = create_matrix_pair(A, B);
    result = matrix_mult_1(&mult_pair, clnt);
    if (result == NULL) {
        clnt_perror(clnt, "call failed");
    } else if (result->success) {
        printf("Multiplication successful!\n");
        print_matrix(&result->result_matrix);
    } else {
        printf("Multiplication failed: %s\n", result->error_msg);
    }
    
    printf("\nClient %d: Testing Matrix Transpose\n", client_id);
    result = matrix_transpose_1(A, clnt);
    if (result == NULL) {
        clnt_perror(clnt, "call failed");
    } else if (result->success) {
        printf("Transpose successful!\n");
        print_matrix(&result->result_matrix);
    } else {
        printf("Transpose failed: %s\n", result->error_msg);
    }
    
    printf("\nClient %d: Testing Matrix Inverse\n", client_id);
    result = matrix_inverse_1(C, clnt);
    if (result == NULL) {
        clnt_perror(clnt, "call failed");
    } else if (result->success) {
        printf("Inverse successful!\n");
        print_matrix(&result->result_matrix);
    } else {
        printf("Inverse failed: %s\n", result->error_msg);
    }
    
    /* Cleanup */
    free(A->data.data_val);
	free(A);
	free(B->data.data_val);
	free(B);
	free(C->data.data_val);
	free(C);
    
    clnt_destroy(clnt);
    
    printf("Client %d: Completed all operations\n", client_id);
    printf("=== Client %d Finished ===\n", client_id);
}

void run_interactive_client(const char *server_address) {
    CLIENT *clnt;
    matrix_result *result;
    int choice;
    
    printf("=== Matrix Operations Client (Interactive Mode) ===\n");
    printf("Connecting to server: %s\n", server_address);
    
    /* Create client handle */
    clnt = clnt_create(server_address, MATRIX_OPERATIONS_PROG, MATRIX_OPERATIONS_VERS, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(server_address);
        printf("Failed to connect to server at %s\n", server_address);
        return;
    }
    
    /* Test connection */
    int *ping_result = ping_1(NULL, clnt);
    if (ping_result == NULL) {
        clnt_perror(clnt, "call failed");
        printf("Server at %s not responding\n", server_address);
        clnt_destroy(clnt);
        return;
    }
    
    printf("Connected to server successfully!\n\n");
    
    while (1) {
        printf("=== Matrix Operations Menu ===\n");
        printf("1. Matrix Addition\n");
        printf("2. Matrix Multiplication\n");
        printf("3. Matrix Transpose\n");
        printf("4. Matrix Inverse\n");
        printf("5. Test Connection\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input! Please enter a number.\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }
        
        if (choice == 0) {
            printf("Goodbye!\n");
            break;
        }
        
        switch (choice) {
            case 1: {
                printf("\n--- Matrix Addition ---\n");
                matrix *A = input_matrix("A");
                matrix *B = input_matrix("B");
                if (A && B) {
                    matrix_pair pair = create_matrix_pair(A, B);
                    result = matrix_add_1(&pair, clnt);
                    if (result == NULL) {
                        printf("RPC call failed!\n");
                    } else if (result->success) {
                        printf("\nAddition Result:\n");
                        print_matrix(&result->result_matrix);
                    } else {
                        printf("Error: %s\n", result->error_msg);
                    }
                    free(A->data.data_val);
					free(A);
					free(B->data.data_val);
					free(B);
                }
                break;
            }
            
            case 2: {
                printf("\n--- Matrix Multiplication ---\n");
                matrix *A = input_matrix("A");
                matrix *B = input_matrix("B");
                if (A && B) {
                    matrix_pair pair = create_matrix_pair(A, B);
                    result = matrix_mult_1(&pair, clnt);
                    if (result == NULL) {
                        printf("RPC call failed!\n");
                    } else if (result->success) {
                        printf("\nMultiplication Result:\n");
                        print_matrix(&result->result_matrix);
                    } else {
                        printf("Error: %s\n", result->error_msg);
                    }
                    free(A->data.data_val);
					free(A);
					free(B->data.data_val);
					free(B);
                }
                break;
            }
            
            case 3: {
                printf("\n--- Matrix Transpose ---\n");
                matrix *A = input_matrix("A");
                if (A) {
                    result = matrix_transpose_1(A, clnt);
                    if (result == NULL) {
                        printf("RPC call failed!\n");
                    } else if (result->success) {
                        printf("\nTranspose Result:\n");
                        print_matrix(&result->result_matrix);
                    } else {
                        printf("Error: %s\n", result->error_msg);
                    }
                    free(A->data.data_val);
                }
                break;
            }
            
            case 4: {
                printf("\n--- Matrix Inverse ---\n");
                matrix *A = input_matrix("A");
                if (A) {
                    result = matrix_inverse_1(A, clnt);
                    if (result == NULL) {
                        printf("RPC call failed!\n");
                    } else if (result->success) {
                        printf("\nInverse Result:\n");
                        print_matrix(&result->result_matrix);
                    } else {
                        printf("Error: %s\n", result->error_msg);
                    }
                    free(A->data.data_val);
                }
                break;
            }
            
            case 5: {
                int *ping_result = ping_1(NULL, clnt);
                if (ping_result && *ping_result) {
                    printf("Server is responding correctly!\n");
                } else {
                    printf("Server connection test failed!\n");
                }
                break;
            }
            
            default:
                printf("Invalid choice! Please try again.\n");
        }
        
        printf("\n");
    }
    
    clnt_destroy(clnt);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage:\n");
        printf("  %s <server_address> test\n", argv[0]);
        printf("  %s <server_address> interactive\n", argv[0]);
        printf("\nExamples:\n");
        printf("  %s localhost test\n", argv[0]);
        printf("  %s 192.168.1.100 interactive\n", argv[0]);
        exit(1);
    }
    
    const char *server_address = argv[1];
    const char *mode = argv[2];
    
    if (strcmp(mode, "test") == 0) {
        printf("Starting automated test to server: %s\n", server_address);
        run_client_test(server_address, 1);
    } else if (strcmp(mode, "interactive") == 0) {
        run_interactive_client(server_address);
    } else {
        printf("Invalid mode: %s\n", mode);
        printf("Use 'test' or 'interactive'\n");
        exit(1);
    }
    
    return 0;
}