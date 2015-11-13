#define ASSERT(a) if (!(a)) { \
                    printf("Assertion failed ==> %s (%s:%d)\n", #a, __FILE__, __LINE__); \
                    exit(0); \
                  }
