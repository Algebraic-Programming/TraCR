#include <ovni.h>
#include "instrumentation.hpp"

/*
Basic test to see if ovni is linked correctly
*/
int main( int argc, char **argv)
{
    printf(OVNI_TRACEDIR, "\n");

    printf("%d\n", INSTRUMENTATION_ACTIVE);

    ovni_version_check();

    return 0;
}