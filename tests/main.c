#include "greatest/greatest.h"

#include "happy_path_tests.h"
#include "encoding_tests.h"
#include "end_to_end_tests.h"
/* #include "bad_input_tests.h" */

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();

  RUN_SUITE(encoding_suite);
  RUN_SUITE(happy_path_suite);
  RUN_SUITE(end_to_end_suite);
  // RUN_SUITE(bad_input_suite);
  
  GREATEST_MAIN_END();
}
