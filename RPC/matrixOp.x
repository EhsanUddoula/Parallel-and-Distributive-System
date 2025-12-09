/* matrixOp.x - Interface Definition Language for Matrix Operations */

const MAX_SIZE = 100;

/* Structure to represent a matrix */
struct matrix {
    int rows;
    int cols;
    double data<MAX_SIZE>;
};

/* Structure for matrix pair (for binary operations) */
struct matrix_pair {
    matrix first;
    matrix second;
};

/* Structure for operation result */
struct matrix_result {
    int success;
    string error_msg<100>;
    matrix result_matrix;
};

/* Program definition */
program MATRIX_OPERATIONS_PROG {
    version MATRIX_OPERATIONS_VERS {
        /* Matrix addition: C = A + B */
        matrix_result MATRIX_ADD(matrix_pair) = 1;
        
        /* Matrix multiplication: C = A * B */
        matrix_result MATRIX_MULT(matrix_pair) = 2;
        
        /* Matrix inverse: B = A^(-1) */
        matrix_result MATRIX_INVERSE(matrix) = 3;
        
        /* Matrix transpose: B = A^T */
        matrix_result MATRIX_TRANSPOSE(matrix) = 4;
        
        /* Test connection */
        int PING(void) = 5;
    } = 1;
} = 0x20000001;