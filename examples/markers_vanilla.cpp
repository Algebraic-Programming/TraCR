#include <chrono>
#include <tracr.hpp>

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

/**
 * Basic c code to allocate memory, run mmm, and free memory.
 * Ovni trace test included with markers.
 * OUTDATED, will be no longer used, only for speed comparisons
 */
int main(void)
{
    std::chrono::time_point<std::chrono::system_clock> start, end, after_label_set;

    start = std::chrono::system_clock::now();

    const size_t N = 4;

    // initialize ovni
    INSTRUMENTATION_START();

    INSTRUMENTATION_VMARKER_TYPE(1, "Simple Marker Example"); // use flag == 1 for push/pop and flag != 1 for set

    INSTRUMENTATION_VMARKER_LABEL(MARK_COLOR_LIGHT_GREEN, "Allocate Memory");    //fyi it is costly (~3us) could be done at the beginning or ending
    INSTRUMENTATION_VMARKER_LABEL(MARK_COLOR_LAVENDER, "Fill matrices with values");
    INSTRUMENTATION_VMARKER_LABEL(MARK_COLOR_MAROON, "Print all matrices");
    INSTRUMENTATION_VMARKER_LABEL(MARK_COLOR_OLIVE, "MMM");
    INSTRUMENTATION_VMARKER_LABEL(MARK_COLOR_NAVY, "Print solution of matrix A");
    INSTRUMENTATION_VMARKER_LABEL(MARK_COLOR_PINK, "Free memory");

    after_label_set = std::chrono::system_clock::now();
    
    // allocate memory
    INSTRUMENTATION_VMARKER_PUSH(MARK_COLOR_LIGHT_GREEN);
    mytype* A = (mytype*) calloc (1, N*N*sizeof(mytype));
    mytype* B = (mytype*) malloc (N*N*sizeof(mytype));
    mytype* C = (mytype*) malloc (N*N*sizeof(mytype));
    INSTRUMENTATION_VMARKER_POP(MARK_COLOR_LIGHT_GREEN);

    // fill matrices
    INSTRUMENTATION_VMARKER_PUSH(MARK_COLOR_LAVENDER);
    for(size_t i = 0; i < N; ++i){
        for(size_t j = 0; j < N; ++j){
            B[i*N + j] = (mytype) i;
            C[i*N + j] = (mytype) j;
        } 
    }
    INSTRUMENTATION_VMARKER_POP(MARK_COLOR_LAVENDER);

    // print matrices
    INSTRUMENTATION_VMARKER_PUSH(MARK_COLOR_MAROON);
    printf("A:\n");
    print_matrix(A, N);

    printf("B:\n");
    print_matrix(B, N);

    printf("C:\n");
    print_matrix(C, N);
    INSTRUMENTATION_VMARKER_POP(MARK_COLOR_MAROON);

    // mmm
    INSTRUMENTATION_VMARKER_PUSH(MARK_COLOR_OLIVE);
    for(size_t i = 0; i < N; ++i){
        for(size_t j = 0; j < N; ++j){
            for(size_t k = 0; k < N; ++k){
                A[i*N + j] += B[i*N + k]*C[k*N + j];
            }
        }    
    }
    INSTRUMENTATION_VMARKER_POP(MARK_COLOR_OLIVE);

    // last print
    INSTRUMENTATION_VMARKER_PUSH(MARK_COLOR_NAVY);
    printf("A (after mmm):\n");
    print_matrix(A, N);
    INSTRUMENTATION_VMARKER_POP(MARK_COLOR_NAVY);

    // free memory
    INSTRUMENTATION_VMARKER_PUSH(MARK_COLOR_PINK);
    free(A);
    free(B);
    free(C);
    INSTRUMENTATION_VMARKER_POP(MARK_COLOR_PINK);

    // ovni fini finish
    INSTRUMENTATION_END();

    end = std::chrono::system_clock::now();

    std::chrono::duration<double> total_time = (end - start);
    
    std::chrono::duration<double> label_set_time = (after_label_set - start);

    std::chrono::duration<double> push_pop_time = (end - after_label_set);

    printf("\n\nTotal time: %f [s]\n", total_time.count());
    
    printf("Label set time: %f [s]\n", label_set_time.count());
    
    printf("Push/Pop time: %f [s]\n", push_pop_time.count());

    return 0;
}