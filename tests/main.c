#include "greatest/greatest.h"

#include "happy_path_tests.h"
/* #include "bad_input_tests.h" */

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();

  RUN_SUITE(happy_path_suite);
  /* RUN_SUITE(bad_input_suite); */
  
  GREATEST_MAIN_END();
}
