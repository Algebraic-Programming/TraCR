#include <chrono>
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
    std::chrono::time_point<std::chrono::system_clock> start, end, after_label_set;

    start = std::chrono::system_clock::now();

    const size_t N = 4;

    // initialize ovni
    INSTRUMENTATION_START();

    INSTRUMENTATION_MARKER_INIT(1);

    const size_t alloc_mem_label_id = INSTRUMENTATION_MARKER_ADD("Allocate Memory", MARK_COLOR_TEAL);          //fyi it is costly (~3us) could be done at the beginning or ending
    const size_t fill_mat_label_id  = INSTRUMENTATION_MARKER_ADD("Fill matrices with values", MARK_COLOR_LAVENDER);
    const size_t prt_mat_label_id   = INSTRUMENTATION_MARKER_ADD("Print all matrices", MARK_COLOR_GRAY);
    const size_t mmm_label_id       = INSTRUMENTATION_MARKER_ADD("MMM", MARK_COLOR_PEACH);
    const size_t prt_A_label_id     = INSTRUMENTATION_MARKER_ADD("Print solution of matrix A", MARK_COLOR_LIGHT_GRAY);
    const size_t free_mem_label_id  = INSTRUMENTATION_MARKER_ADD("Free memory", MARK_COLOR_MINT);

    after_label_set = std::chrono::system_clock::now();

    // allocate memory
    INSTRUMENTATION_MARKER_PUSH(alloc_mem_label_id);
    mytype* A = (mytype*) calloc (1, N*N*sizeof(mytype));
    mytype* B = (mytype*) malloc (N*N*sizeof(mytype));
    mytype* C = (mytype*) malloc (N*N*sizeof(mytype));
    INSTRUMENTATION_MARKER_POP(alloc_mem_label_id);

    // fill matrices
    INSTRUMENTATION_MARKER_PUSH(fill_mat_label_id);
    for(size_t i = 0; i < N; ++i){
        for(size_t j = 0; j < N; ++j){
            B[i*N + j] = (mytype) i;
            C[i*N + j] = (mytype) j;
        } 
    }
    INSTRUMENTATION_MARKER_POP(fill_mat_label_id);

    // print matrices
    INSTRUMENTATION_MARKER_PUSH(prt_mat_label_id);
    printf("A:\n");
    print_matrix(A, N);

    printf("B:\n");
    print_matrix(B, N);

    printf("C:\n");
    print_matrix(C, N);
    INSTRUMENTATION_MARKER_POP(prt_mat_label_id);

    // mmm
    INSTRUMENTATION_MARKER_PUSH(mmm_label_id);
    for(size_t i = 0; i < N; ++i){
        for(size_t j = 0; j < N; ++j){
            for(size_t k = 0; k < N; ++k){
                A[i*N + j] += B[i*N + k]*C[k*N + j];
            }
        }    
    }
    INSTRUMENTATION_MARKER_POP(mmm_label_id);

    // last print
    INSTRUMENTATION_MARKER_PUSH(prt_A_label_id);
    printf("A (after mmm):\n");
    print_matrix(A, N);
    INSTRUMENTATION_MARKER_POP(prt_A_label_id);
    
    // free memory
    INSTRUMENTATION_MARKER_PUSH(free_mem_label_id);
    free(A);
    free(B);
    free(C);
    INSTRUMENTATION_MARKER_POP(free_mem_label_id);

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