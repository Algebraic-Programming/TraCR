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
    const size_t N = 4;

    // initialize ovni
    INSTRUMENTATION_START();

    INSTRUMENTATION_MARKER_INIT(0);

    INSTRUMENTATION_MARKER_ADD("Allocate Memory", MARK_COLOR_TEAL);          //fyi it is costly (~3us) could be done at the beginning or ending
    INSTRUMENTATION_MARKER_ADD("Fill matrices with values", MARK_COLOR_LAVENDER);
    INSTRUMENTATION_MARKER_ADD("Print all matrices", MARK_COLOR_GRAY);
    INSTRUMENTATION_MARKER_ADD("MMM", MARK_COLOR_PEACH);
    INSTRUMENTATION_MARKER_ADD("Print solution of matrix A", MARK_COLOR_LIGHT_GRAY);
    INSTRUMENTATION_MARKER_ADD("Free memory", MARK_COLOR_MINT);
    INSTRUMENTATION_MARKER_ADD("Done", MARK_COLOR_BROWN);


    // allocate memory
    INSTRUMENTATION_MARKER_SET("Allocate Memory");
    mytype* A = (mytype*) calloc (1, N*N*sizeof(mytype));
    mytype* B = (mytype*) malloc (N*N*sizeof(mytype));
    mytype* C = (mytype*) malloc (N*N*sizeof(mytype));

    // fill matrices
    INSTRUMENTATION_MARKER_SET("Fill matrices with values");
    for(size_t i = 0; i < N; ++i){
        for(size_t j = 0; j < N; ++j){
            B[i*N + j] = (mytype) i;
            C[i*N + j] = (mytype) j;
        } 
    }

    // print matrices
    INSTRUMENTATION_MARKER_SET("Print all matrices");
    printf("A:\n");
    print_matrix(A, N);

    printf("B:\n");
    print_matrix(B, N);

    printf("C:\n");
    print_matrix(C, N);

    // mmm
    INSTRUMENTATION_MARKER_SET("MMM");
    for(size_t i = 0; i < N; ++i){
        for(size_t j = 0; j < N; ++j){
            for(size_t k = 0; k < N; ++k){
                A[i*N + j] += B[i*N + k]*C[k*N + j];
            }
        }    
    }

    // last print
    INSTRUMENTATION_MARKER_SET("Print solution of matrix A");
    printf("A (after mmm):\n");
    print_matrix(A, N);
    
    // free memory
    INSTRUMENTATION_MARKER_SET("Free memory");
    free(A);
    free(B);
    free(C);

    INSTRUMENTATION_MARKER_SET("Done");

    // ovni fini finish
    INSTRUMENTATION_END();

    return 0;
}