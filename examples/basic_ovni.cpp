#include <ovni.h>
#include <tracr.hpp>

/*
Basic test to see if ovni is linked correctly
*/
int main(int argc, char **argv) {
  printf("OVNI_TRACEDIR: %s\n", OVNI_TRACEDIR);

  printf("Instrumentation enabled: %d\n", INSTRUMENTATION_ACTIVE);

  ovni_version_check();

  return 0;
}
