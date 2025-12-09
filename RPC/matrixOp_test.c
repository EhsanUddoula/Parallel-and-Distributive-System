/*
 * matrixOp_test.c - Comprehensive test suite for Matrix RPC Operations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "matrixOp.h"

#define ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("❌ FAIL: %s\n", message); \
            failures++; \
        } else { \
            printf("✅ PASS: %s\n", message); \
            successes++; \
        } \
    } while(0)

#define EPSILON 1e-6

int successes = 0;
int failures = 0;

/* Helper function to create test matrix */
matrix *create_test_matrix_data(int rows, int cols, double *data) {
    matrix *mat = (matrix *)malloc(sizeof(matrix));
    if (!mat) return NULL;
    
    mat->rows = rows;
    mat->cols = cols;
    mat->data.data_len = rows * cols;
    mat->data.data_val = (double *)malloc(rows * cols * sizeof(double));
    
    if (data) {
        memcpy(mat->data.data_val, data, rows * cols * sizeof(double));
    }
    
    return mat;
}

/* Helper to compare two matrices */
int matrices_equal(const matrix *a, const matrix *b, double tolerance) {
    if (a->rows != b->rows || a->cols != b->cols) return 0;
    
    for (int i = 0; i < a->rows * a->cols; i++) {
        if (fabs(a->data.data_val[i] - b->data.data_val[i]) > tolerance) {
            return 0;
        }
    }
    return 1;
}

/* Print matrix for debugging */
void print_matrix_debug(const matrix *mat, const char *name) {
    printf("%s (%dx%d):\n", name, mat->rows, mat->cols);
    for (int i = 0; i < mat->rows; i++) {
        printf("  ");
        for (int j = 0; j < mat->cols; j++) {
            printf("%8.3f ", mat->data.data_val[i * mat->cols + j]);
        }
        printf("\n");
    }
}

/* Test 1: Basic Matrix Addition */
void test_matrix_addition(CLIENT *clnt) {
    printf("\n=== Test 1: Matrix Addition ===\n");
    
    // Test case 1.1: 2x2 addition
    double dataA1[] = {1, 2, 3, 4};
    double dataB1[] = {5, 6, 7, 8};
    double expected1[] = {6, 8, 10, 12};
    
    matrix *A1 = create_test_matrix_data(2, 2, dataA1);
    matrix *B1 = create_test_matrix_data(2, 2, dataB1);
    matrix *expected_mat1 = create_test_matrix_data(2, 2, expected1);
    
    matrix_pair pair1;
    pair1.first = *A1;
    pair1.second = *B1;
    
    matrix_result *result1 = matrix_add_1(&pair1, clnt);
    ASSERT(result1 != NULL, "Addition RPC call should not return NULL");
    ASSERT(result1->success, "Addition should succeed");
    ASSERT(matrices_equal(&result1->result_matrix, expected_mat1, EPSILON), 
           "2x2 addition result should be correct");
    
    free(A1->data.data_val); free(A1);
    free(B1->data.data_val); free(B1);
    free(expected_mat1->data.data_val); free(expected_mat1);
    
    // Test case 1.2: 3x3 addition
    double dataA2[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    double dataB2[] = {9, 8, 7, 6, 5, 4, 3, 2, 1};
    double expected2[] = {10, 10, 10, 10, 10, 10, 10, 10, 10};
    
    matrix *A2 = create_test_matrix_data(3, 3, dataA2);
    matrix *B2 = create_test_matrix_data(3, 3, dataB2);
    matrix *expected_mat2 = create_test_matrix_data(3, 3, expected2);
    
    matrix_pair pair2;
    pair2.first = *A2;
    pair2.second = *B2;
    
    matrix_result *result2 = matrix_add_1(&pair2, clnt);
    ASSERT(result2 != NULL, "3x3 Addition RPC call should not return NULL");
    ASSERT(result2->success, "3x3 Addition should succeed");
    ASSERT(matrices_equal(&result2->result_matrix, expected_mat2, EPSILON), 
           "3x3 addition result should be correct");
    
    free(A2->data.data_val); free(A2);
    free(B2->data.data_val); free(B2);
    free(expected_mat2->data.data_val); free(expected_mat2);
}

/* Test 2: Matrix Multiplication */
void test_matrix_multiplication(CLIENT *clnt) {
    printf("\n=== Test 2: Matrix Multiplication ===\n");
    
    // Test case 2.1: 2x2 multiplication
    double dataA1[] = {1, 2, 3, 4};
    double dataB1[] = {5, 6, 7, 8};
    double expected1[] = {19, 22, 43, 50};
    
    matrix *A1 = create_test_matrix_data(2, 2, dataA1);
    matrix *B1 = create_test_matrix_data(2, 2, dataB1);
    matrix *expected_mat1 = create_test_matrix_data(2, 2, expected1);
    
    matrix_pair pair1;
    pair1.first = *A1;
    pair1.second = *B1;
    
    matrix_result *result1 = matrix_mult_1(&pair1, clnt);
    ASSERT(result1 != NULL, "Multiplication RPC call should not return NULL");
    ASSERT(result1->success, "Multiplication should succeed");
    ASSERT(matrices_equal(&result1->result_matrix, expected_mat1, EPSILON), 
           "2x2 multiplication result should be correct");
    
    free(A1->data.data_val); free(A1);
    free(B1->data.data_val); free(B1);
    free(expected_mat1->data.data_val); free(expected_mat1);
    
    // Test case 2.2: 2x3 * 3x2 multiplication
    double dataA2[] = {1, 2, 3, 4, 5, 6};
    double dataB2[] = {7, 8, 9, 10, 11, 12};
    double expected2[] = {58, 64, 139, 154};
    
    matrix *A2 = create_test_matrix_data(2, 3, dataA2);
    matrix *B2 = create_test_matrix_data(3, 2, dataB2);
    matrix *expected_mat2 = create_test_matrix_data(2, 2, expected2);
    
    matrix_pair pair2;
    pair2.first = *A2;
    pair2.second = *B2;
    
    matrix_result *result2 = matrix_mult_1(&pair2, clnt);
    ASSERT(result2 != NULL, "2x3*3x2 Multiplication RPC call should not return NULL");
    ASSERT(result2->success, "2x3*3x2 Multiplication should succeed");
    ASSERT(matrices_equal(&result2->result_matrix, expected_mat2, EPSILON), 
           "2x3*3x2 multiplication result should be correct");
    
    free(A2->data.data_val); free(A2);
    free(B2->data.data_val); free(B2);
    free(expected_mat2->data.data_val); free(expected_mat2);
}

/* Test 3: Matrix Transpose */
void test_matrix_transpose(CLIENT *clnt) {
    printf("\n=== Test 3: Matrix Transpose ===\n");
    
    // Test case 3.1: 2x3 transpose
    double dataA1[] = {1, 2, 3, 4, 5, 6};
    double expected1[] = {1, 4, 2, 5, 3, 6};
    
    matrix *A1 = create_test_matrix_data(2, 3, dataA1);
    matrix *expected_mat1 = create_test_matrix_data(3, 2, expected1);
    
    matrix_result *result1 = matrix_transpose_1(A1, clnt);
    ASSERT(result1 != NULL, "Transpose RPC call should not return NULL");
    ASSERT(result1->success, "Transpose should succeed");
    ASSERT(matrices_equal(&result1->result_matrix, expected_mat1, EPSILON), 
           "2x3 transpose result should be correct");
    
    free(A1->data.data_val); free(A1);
    free(expected_mat1->data.data_val); free(expected_mat1);
}

/* Test 4: Matrix Inverse */
void test_matrix_inverse(CLIENT *clnt) {
    printf("\n=== Test 4: Matrix Inverse ===\n");
    
    // Test case 4.1: 2x2 inverse
    double dataA1[] = {4, 7, 2, 6};
    double expected1[] = {0.6, -0.7, -0.2, 0.4};
    
    matrix *A1 = create_test_matrix_data(2, 2, dataA1);
    matrix *expected_mat1 = create_test_matrix_data(2, 2, expected1);
    
    matrix_result *result1 = matrix_inverse_1(A1, clnt);
    ASSERT(result1 != NULL, "Inverse RPC call should not return NULL");
    ASSERT(result1->success, "2x2 Inverse should succeed");
    ASSERT(matrices_equal(&result1->result_matrix, expected_mat1, EPSILON), 
           "2x2 inverse result should be correct");
    
    free(A1->data.data_val); free(A1);
    free(expected_mat1->data.data_val); free(expected_mat1);
    
    // Test case 4.2: 3x3 inverse
    double dataA2[] = {2, -1, 0, -1, 2, -1, 0, -1, 2};
    double expected2[] = {0.75, 0.5, 0.25, 0.5, 1.0, 0.5, 0.25, 0.5, 0.75};
    
    matrix *A2 = create_test_matrix_data(3, 3, dataA2);
    matrix *expected_mat2 = create_test_matrix_data(3, 3, expected2);
    
    matrix_result *result2 = matrix_inverse_1(A2, clnt);
    ASSERT(result2 != NULL, "3x3 Inverse RPC call should not return NULL");
    ASSERT(result2->success, "3x3 Inverse should succeed");
    // Use larger epsilon for 3x3 due to floating point precision
    ASSERT(matrices_equal(&result2->result_matrix, expected_mat2, 1e-4), 
           "3x3 inverse result should be correct");
    
    free(A2->data.data_val); free(A2);
    free(expected_mat2->data.data_val); free(expected_mat2);
}

/* Test 5: Error Conditions */
void test_error_conditions(CLIENT *clnt) {
    printf("\n=== Test 5: Error Conditions ===\n");
    
    // Test case 5.1: Dimension mismatch for addition
    double dataA1[] = {1, 2, 3, 4};
    double dataB1[] = {1, 2};
    
    matrix *A1 = create_test_matrix_data(2, 2, dataA1);
    matrix *B1 = create_test_matrix_data(1, 2, dataB1);
    
    matrix_pair pair1;
    pair1.first = *A1;
    pair1.second = *B1;
    
    matrix_result *result1 = matrix_add_1(&pair1, clnt);
    ASSERT(result1 != NULL, "Invalid addition RPC call should not return NULL");
    ASSERT(!result1->success, "Addition with dimension mismatch should fail");
    ASSERT(strstr(result1->error_msg, "same dimensions") != NULL, 
           "Should return appropriate error message");
    
    free(A1->data.data_val); free(A1);
    free(B1->data.data_val); free(B1);
    
    // Test case 5.2: Non-square matrix inverse
    double dataA2[] = {1, 2, 3, 4, 5, 6};
    matrix *A2 = create_test_matrix_data(2, 3, dataA2);
    
    matrix_result *result2 = matrix_inverse_1(A2, clnt);
    ASSERT(result2 != NULL, "Non-square inverse RPC call should not return NULL");
    ASSERT(!result2->success, "Non-square matrix inverse should fail");
    ASSERT(strstr(result2->error_msg, "square") != NULL, 
           "Should return appropriate error message");
    
    free(A2->data.data_val); free(A2);
}

/* Test 6: Connection Test */
void test_connection(CLIENT *clnt) {
    printf("\n=== Test 6: Connection Test ===\n");
    
    int *ping_result = ping_1(NULL, clnt);
    ASSERT(ping_result != NULL, "Ping RPC call should not return NULL");
    ASSERT(*ping_result == 1, "Ping should return 1 (success)");
    
    printf("✅ PASS: Server connection successful\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <server_address>\n", argv[0]);
        printf("Example: %s localhost\n", argv[0]);
        exit(1);
    }
    
    const char *server_address = argv[1];
    
    printf("========================================\n");
    printf("   Matrix RPC Test Suite\n");
    printf("   Server: %s\n", server_address);
    printf("========================================\n");
    
    // Create client handle
    CLIENT *clnt = clnt_create(server_address, MATRIX_OPERATIONS_PROG, MATRIX_OPERATIONS_VERS, "tcp");
    if (clnt == NULL) {
        clnt_pcreateerror(server_address);
        printf("❌ FAILED: Cannot connect to server at %s\n", server_address);
        exit(1);
    }
    
    // Run all tests
    test_connection(clnt);
    test_matrix_addition(clnt);
    test_matrix_multiplication(clnt);
    test_matrix_transpose(clnt);
    test_matrix_inverse(clnt);
    test_error_conditions(clnt);
    
    // Print summary
    printf("\n========================================\n");
    printf("   TEST SUMMARY\n");
    printf("========================================\n");
    printf("Total Tests: %d\n", successes + failures);
    printf("✅ Passed: %d\n", successes);
    printf("❌ Failed: %d\n", failures);
    printf("Success Rate: %.1f%%\n", (successes * 100.0) / (successes + failures));
    
    if (failures == 0) {
        printf("\n🎉 ALL TESTS PASSED! 🎉\n");
    } else {
        printf("\n⚠️  SOME TESTS FAILED! ⚠️\n");
    }
    
    clnt_destroy(clnt);
    return failures == 0 ? 0 : 1;
}