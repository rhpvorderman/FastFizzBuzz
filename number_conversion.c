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

static int write_number(uint64_t number, size_t number_of_decimals, char *restrict buffer) {
    if (number_of_decimals > 20) {
        return -1;
    }
    size_t buffer_pos = 0;
    switch (number_of_decimals) {
        case 20:
            buffer[buffer_pos] = (number / TEN_TO_THE_19) + '0';
            buffer_pos += 1;
        case 19:
            buffer[buffer_pos] = (number / TEN_TO_THE_18) % 10 + '0';
            buffer_pos += 1;
        case 18:
            buffer[buffer_pos] = (number / TEN_TO_THE_17) % 10 + '0';
            buffer_pos += 1;
        case 17:
            buffer[buffer_pos] = (number / TEN_TO_THE_16) % 10 + '0';
            buffer_pos += 1;
        case 16:
            buffer[buffer_pos] = (number / TEN_TO_THE_15) % 10 + '0';
            buffer_pos += 1;
        case 15:
            buffer[buffer_pos] = (number / TEN_TO_THE_14) % 10 + '0';
            buffer_pos += 1;
        case 14:
            buffer[buffer_pos] = (number / TEN_TO_THE_13) % 10 + '0';
            buffer_pos += 1;
        case 13:
            buffer[buffer_pos] = (number / TEN_TO_THE_12) % 10 + '0';
            buffer_pos += 1;
        case 12:
            buffer[buffer_pos] = (number / TEN_TO_THE_11) % 10 + '0';
            buffer_pos += 1;
        case 11:
            buffer[buffer_pos] = (number / TEN_TO_THE_10) % 10 + '0';
            buffer_pos += 1;
        case 10:
            buffer[buffer_pos] = (number / TEN_TO_THE_9) % 10 + '0';
            buffer_pos += 1;
        case 9:
            buffer[buffer_pos] = (number / TEN_TO_THE_8) % 10 + '0';
            buffer_pos += 1;
        case 8:
            buffer[buffer_pos] = (number / TEN_TO_THE_7) % 10 + '0';
            buffer_pos += 1;
        case 7:
            buffer[buffer_pos] = (number / TEN_TO_THE_6) % 10 + '0';
            buffer_pos += 1;
        case 6:
            buffer[buffer_pos] = (number / TEN_TO_THE_5) % 10 + '0';
            buffer_pos += 1;
        case 5:
            buffer[buffer_pos] = (number / TEN_TO_THE_4) % 10 + '0';
            buffer_pos += 1;
        case 4:
            buffer[buffer_pos] = (number / TEN_TO_THE_3) % 10 + '0';
            buffer_pos += 1;
        case 3:
            buffer[buffer_pos] = (number / TEN_TO_THE_2) % 10 + '0';
            buffer_pos += 1;
        case 2:
            buffer[buffer_pos] = (number / TEN_TO_THE_1) % 10 + '0';
            buffer_pos += 1;
        case 1:
            buffer[buffer_pos] = (number / TEN_TO_THE_0) % 10 + '0';
            buffer_pos += 1;
        case 0:
            buffer[buffer_pos] = '\n';
            buffer_pos += 1;
    }
    return buffer_pos;
}

int main() {
    char buffer[BUFFER_SIZE];
    size_t buffer_size = 0;
    size_t number_of_decimals = 1;
    size_t threshold_stage = 0;
    uint64_t threshold = THRESHOLDS[threshold_stage];
    for (uint64_t i=1; i < FIZZBUZZ_LIMIT; i++) {
        if (i > threshold) {
            threshold_stage += 1;
            threshold = THRESHOLDS[threshold_stage];
            number_of_decimals += 1;
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
            buffer_size += write_number(i, number_of_decimals, buffer + buffer_size);
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
