#include <detectr.hpp>

#define mytype float

/**
 * print the square matrices nice
 */
static void print_matrix(mytype* matrix, const size_t N)
{
    for(size_t i = 0; i < N; ++i){
        for(size_t j = 0; j < N; ++j){
            printf("%f ", matrix[i*N + j]);
        }
        printf("\n");    
    }
    printf("\n");
}

/*
Basic c code to allocate memory, run mmm, and free memory.
Ovni trace test included with markers.
*/
int main(void)
{
    // initialize ovni
    INSTRUMENTATION_START();

    const size_t N = 4;

    int64_t value = 1;
    int32_t type = 0;
    long flag = OVNI_MARK_STACK;      // use flag == 1 for push/pop and flag != 1 for set

    INSTRUMENTATION_MARK_TYPE(type, flag, "Hello World");
    
    // allocate memory
    INSTRUMENTATION_MARK_LABEL(type, value, "Allocate Memory");    //fyi it is costly (~3us) could be done at the beginning or ending
    INSTRUMENTATION_MARK_PUSH(type, value);
    mytype* A = (mytype*) calloc (1, N*N*sizeof(mytype));
    mytype* B = (mytype*) malloc (N*N*sizeof(mytype));
    mytype* C = (mytype*) malloc (N*N*sizeof(mytype));
    INSTRUMENTATION_MARK_POP(type, value);

    ++value;

    // fill matrices
    INSTRUMENTATION_MARK_LABEL(type, value, "Fill matrices with values");
    INSTRUMENTATION_MARK_PUSH(type, value);
    for(size_t i = 0; i < N; ++i){
        for(size_t j = 0; j < N; ++j){
            B[i*N + j] = (mytype) i;
            C[i*N + j] = (mytype) j;
        } 
    }
    INSTRUMENTATION_MARK_POP(type, value);

    ++value;

    // print matrices
    INSTRUMENTATION_MARK_LABEL(type, value, "Print all matrices");
    INSTRUMENTATION_MARK_PUSH(type, value);
    printf("A:\n");
    print_matrix(A, N);

    printf("B:\n");
    print_matrix(B, N);

    printf("C:\n");
    print_matrix(C, N);
    INSTRUMENTATION_MARK_POP(type, value);

    ++value;

    // mmm
    INSTRUMENTATION_MARK_LABEL(type, value, "MMM");
    INSTRUMENTATION_MARK_PUSH(type, value);
    for(size_t i = 0; i < N; ++i){
        for(size_t j = 0; j < N; ++j){
            for(size_t k = 0; k < N; ++k){
                A[i*N + j] += B[i*N + k]*C[k*N + j];
            }
        }    
    }
    INSTRUMENTATION_MARK_POP(type, value);

    ++value;

    // last print
    INSTRUMENTATION_MARK_LABEL(type, value, "Print solution of matrix A");
    INSTRUMENTATION_MARK_PUSH(type, value);
    printf("A (after mmm):\n");
    print_matrix(A, N);
    INSTRUMENTATION_MARK_POP(type, value);

    ++value;

    // free memory
    INSTRUMENTATION_MARK_LABEL(type, value, "Free memory");
    INSTRUMENTATION_MARK_PUSH(type, value);
    free(A);
    free(B);
    free(C);
    INSTRUMENTATION_MARK_POP(type, value);

    // ovni fini finish
    INSTRUMENTATION_END();

    return 0;
}