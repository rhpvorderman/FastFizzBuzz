#include <stdio.h>
#include <stdint.h>

#define FIZZBUZZ_LIMIT (1ULL << 32ULL)
int main() {
    for (uint64_t i=1; i < FIZZBUZZ_LIMIT; i++) {
        if (i % 15 == 0) {
            printf("FizzBuzz\n");
        }
        else if (i % 3 == 0) {
            printf("Fizz\n");
        }
        else if (i % 5 == 0) {
            printf("Buzz\n");
        }
        else {
            printf("%lu\n", i);
        }
    }
    return 0;
}
