#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define FIZZBUZZ_LIMIT (1ULL << 32ULL)
#define BUFFER_SIZE 32768
#define BUFFER_LIMIT 32700

#define TEN_TO_THE_0  1ULL
#define TEN_TO_THE_1  10ULL
#define TEN_TO_THE_2  100ULL
#define TEN_TO_THE_3  1000ULL
#define TEN_TO_THE_4  10000ULL
#define TEN_TO_THE_5  100000ULL
#define TEN_TO_THE_6  1000000ULL
#define TEN_TO_THE_7  10000000ULL
#define TEN_TO_THE_8  100000000ULL
#define TEN_TO_THE_9  1000000000ULL
#define TEN_TO_THE_10 10000000000ULL
#define TEN_TO_THE_11 100000000000ULL
#define TEN_TO_THE_12 1000000000000ULL
#define TEN_TO_THE_13 10000000000000ULL
#define TEN_TO_THE_14 100000000000000ULL
#define TEN_TO_THE_15 1000000000000000ULL
#define TEN_TO_THE_16 10000000000000000ULL
#define TEN_TO_THE_17 100000000000000000ULL
#define TEN_TO_THE_18 1000000000000000000ULL
#define TEN_TO_THE_19 10000000000000000000ULL

static uint64_t THRESHOLDS[19] = {
    TEN_TO_THE_1,
    TEN_TO_THE_2,
    TEN_TO_THE_3,
    TEN_TO_THE_4,
    TEN_TO_THE_5,
    TEN_TO_THE_6,
    TEN_TO_THE_7,
    TEN_TO_THE_8,
    TEN_TO_THE_9,
    TEN_TO_THE_10,
    TEN_TO_THE_11,
    TEN_TO_THE_12,
    TEN_TO_THE_13,
    TEN_TO_THE_14,
    TEN_TO_THE_15,
    TEN_TO_THE_16,
    TEN_TO_THE_17,
    TEN_TO_THE_18,
    TEN_TO_THE_19
};

static int write_number(uint64_t number, size_t number_of_digits, char *restrict buffer) {
    if (number_of_digits > 20) {
        return -1;
    }
    /* All the statements below are independent. As a result it should be easy
       for the CPU to pipeline them. */    
    switch (number_of_digits) {
        case 20:
            buffer[number_of_digits - 20] = (number / TEN_TO_THE_19) + '0';
        case 19:
            buffer[number_of_digits - 19] = (number / TEN_TO_THE_18) % 10 + '0';
        case 18:
            buffer[number_of_digits - 18] = (number / TEN_TO_THE_17) % 10 + '0';
        case 17:
            buffer[number_of_digits - 17] = (number / TEN_TO_THE_16) % 10 + '0';
        case 16:
            buffer[number_of_digits - 16] = (number / TEN_TO_THE_15) % 10 + '0';
        case 15:
            buffer[number_of_digits - 15] = (number / TEN_TO_THE_14) % 10 + '0';
        case 14:
            buffer[number_of_digits - 14] = (number / TEN_TO_THE_13) % 10 + '0';
        case 13:
            buffer[number_of_digits - 13] = (number / TEN_TO_THE_12) % 10 + '0';
        case 12:
            buffer[number_of_digits - 12] = (number / TEN_TO_THE_11) % 10 + '0';
        case 11:
            buffer[number_of_digits - 11] = (number / TEN_TO_THE_10) % 10 + '0';
        case 10:
            buffer[number_of_digits - 10] = (number / TEN_TO_THE_9) % 10 + '0';
        case 9:
            buffer[number_of_digits - 9] = (number / TEN_TO_THE_8) % 10 + '0';
        case 8:
            buffer[number_of_digits - 8] = (number / TEN_TO_THE_7) % 10 + '0';
        case 7:
            buffer[number_of_digits - 7] = (number / TEN_TO_THE_6) % 10 + '0';
        case 6:
            buffer[number_of_digits - 6] = (number / TEN_TO_THE_5) % 10 + '0';
        case 5:
            buffer[number_of_digits - 5] = (number / TEN_TO_THE_4) % 10 + '0';
        case 4:
            buffer[number_of_digits - 4] = (number / TEN_TO_THE_3) % 10 + '0';
        case 3:
            buffer[number_of_digits - 3] = (number / TEN_TO_THE_2) % 10 + '0';
        case 2:
            buffer[number_of_digits - 2] = (number / TEN_TO_THE_1) % 10 + '0';
        case 1:
            buffer[number_of_digits - 1] = (number / TEN_TO_THE_0) % 10 + '0';
        case 0:
            buffer[number_of_digits] = '\n';
    }
    return number_of_digits + 1;
}

int main() {
    char buffer[BUFFER_SIZE];
    size_t buffer_size = 0;
    size_t number_of_digits = 1;
    size_t threshold_stage = 0;
    uint64_t threshold = THRESHOLDS[threshold_stage];
    for (uint64_t i=1; i < FIZZBUZZ_LIMIT; i++) {
        if (i > threshold) {
            threshold_stage += 1;
            threshold = THRESHOLDS[threshold_stage];
            number_of_digits += 1;
        }
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
            buffer_size += write_number(i, number_of_digits, buffer + buffer_size);
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
