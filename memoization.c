#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

#define FIZZBUZZ_LIMIT (1ULL << 32ULL)
#define BUFFER_SIZE 32768

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

static inline size_t calculate_number_of_digits(uint64_t number) {
    /* Since the range between 2^x and 2^(x+1) often uses the same number of 
       digits, we can use the leading_zero_count and a lookup table to get the 
       number of decimal digits very quickly. For ambigious cases there are 
       only two possibilities which can be found by looking up the correct 
       threshold and see if the number is below it. */
    static char leading_zeros_to_number_of_digits[65] = {
        -19, 19, 19, 19, -18, 18, 18, -17, 17, 17, -16, 16, 16, 16, -15, 15,
        15, -14, 14, 14, -13, 13, 13, 13, -12, 12, 12, -11, 11, 11, -10, 10,
        10, 10, -9, 9, 9, -8, 8, 8, -7, 7, 7, 7, -6, 6, 6, -5, 5, 5, -4, 4, 4,
        4, -3, 3, 3, -2, 2, 2, -1, 1, 1, 1, 1
    };
    size_t leading_zero_count = __lzcnt64(number);
    char number_of_digits = leading_zeros_to_number_of_digits[leading_zero_count];
    if (number_of_digits > 0) {
        return number_of_digits;
    }
    size_t threshold_index = -number_of_digits - 1;
    if (number < THRESHOLDS[threshold_index]) {
        return -number_of_digits;
    }
    return -number_of_digits + 1;
}

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

static size_t fizzbuzz(uint64_t start, uint64_t stop, char *buffer) {
    size_t buffer_size = 0;
    for (uint64_t i=start; i < stop; i++) {

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
            size_t number_of_digits = calculate_number_of_digits(i);
            buffer_size += write_number(i, number_of_digits, buffer + buffer_size);
        }
    }   
    return buffer_size;
}

static char zero_to_999[4000];

static void initialize_zero_to_999(void) {
    for (size_t i=0; i<10; i++) {
        for (size_t j=0; j<10; j++) {
            for (size_t k=0; k<10; k++) {
                size_t pos = (i * 100 + j * 10 + k * 1) * 4;
                zero_to_999[pos + 0] = i + '0';
                zero_to_999[pos + 1] = j + '0';
                zero_to_999[pos + 2] = k + '0';
                zero_to_999[pos + 3] = '\n';
            }
        }
    }
}

static size_t fizzbuzz_memoized(uint64_t start, uint64_t stop, char *buffer) {
    if (start % 1000 != 0 || stop - start > 1000) {
        fprintf(
            stderr, 
            "Assumption for memoization broken, start must be divisible by "
            "1000, stop must be at most 1000 positions away from start.\n"
            "Start: %lu; Stop: %lu\n", start, stop
        );
        exit(1);
    }
    size_t number_of_digits = calculate_number_of_digits(start);
    size_t buffer_size = 0;
    char prefix[21];
    write_number(start, number_of_digits, prefix);
    size_t prefix_length = number_of_digits - 3;
    size_t index = 0;
    for (uint64_t i=start; i < stop; i++) {

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
            memcpy(buffer + buffer_size, prefix, 20);
            buffer_size += prefix_length;
            /* Lookup the last 3 digits. */
            size_t pos = index * 4;
            memcpy(buffer + buffer_size, zero_to_999 + pos, 4);
            buffer_size += 4;
        }
        index += 1;
    }   
    return buffer_size;
}
static inline uint64_t uint64_min(uint64_t a, uint64_t b) {
    if (a < b) {
        return a;
    }
    return b;
}

int main() {
    char buffer[BUFFER_SIZE];
    initialize_zero_to_999();
    uint64_t start = 1;
    uint64_t end = FIZZBUZZ_LIMIT;

    /* First calculate the result up to 1000 */
    uint64_t stop = uint64_min(1000, end);
    size_t buffer_size = fizzbuzz(start, stop, buffer);
    fwrite(buffer, 1, buffer_size, stdout);
    start = 1000;
    if (end <= 1000) {
        fflush(stdout);
        return 0;
    }
    while (start < end) {
        stop = uint64_min(start + 1000, end);
        buffer_size = fizzbuzz_memoized(start, stop, buffer);
        fwrite(buffer, 1, buffer_size, stdout);
        start = stop;
    }
    fflush(stdout);
    return 0;
}
