#define mu_assert(test, message) do { \
  if (!(test)) { \
    return message; \
  } \
} while (0)

#define mu_run_test(test) do { \
  before_each(); \
  char *message = test(); \
  after_each(); \
  tests_run++; \
  if (message) { \
    return message; \
  } \
  tests_passed++; \
} while (0)

extern int tests_run;
extern int tests_passed;