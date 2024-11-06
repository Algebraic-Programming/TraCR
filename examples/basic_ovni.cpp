#include <ovni.h>

/*
Basic test to see if ovni is linked correctly
*/
int main( int argc, char **argv)
{
    printf(OVNI_TRACEDIR);

    printf("\n");

    ovni_version_check();

    return 0;
}