#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define FIZZBUZZ_LIMIT (1ULL << 32ULL)
#define BUFFER_SIZE 32768
#define BUFFER_LIMIT 32700

int main() {
    char buffer[BUFFER_SIZE];
    size_t buffer_size = 0;
    for (uint64_t i=1; i < FIZZBUZZ_LIMIT; i++) {
        if (i % 15 == 0) {
            memcpy(buffer + buffer_size, "FizzBuzz\n", 9);
            buffer_size += 9;
        }
        else if (i % 3 == 0) {
            memcpy(buffer + buffer_size, "Fizz\n", 5);
            buffer_size += 5;
        }
        else if (i % 5 == 0) {
            memcpy(buffer + buffer_size, "Buzz\n", 5);
            buffer_size += 5;
        }
        else {
            buffer_size += sprintf(buffer+buffer_size, "%lu\n", i);
        }
        if (buffer_size > BUFFER_LIMIT) {
            fwrite(buffer, 1, buffer_size, stdout);
            buffer_size = 0;
        }
    }
    fwrite(buffer, 1, buffer_size, stdout);
    fflush(stdout);
    return 0;
}
