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

static inline size_t number_from_prefix_and_index(
    const char *restrict prefix, size_t prefix_length, size_t index, 
    char * buffer) 
{
    /* Using a fixed memcpy size avoids a function call. The largest 
        integer is 20 digits in size.*/
    memcpy(buffer, prefix, 20);
        buffer += prefix_length;
        /* Lookup the last 3 digits. */
        size_t pos = index * 4;
        memcpy(buffer, zero_to_999 + pos, 4);
        return prefix_length + 4;
}

static void initialize_unroll_template(
    char *unroll_template, 
    char *template_number, 
    size_t number_of_digits
) {
        /* Using a fixed memcpy size avoids a function call. The largest 
           integer is 20 digits in size.*/
        char *cursor = unroll_template;
        memcpy(unroll_template, template_number, 20);  // 1
        unroll_template[number_of_digits] = '\n';
        cursor += number_of_digits + 1;
        memcpy(cursor, template_number, 20);  // 2
        cursor += number_of_digits + 1;
        memcpy(cursor, "Fizz\n", 5); // 3
        cursor += 5;
        memcpy(cursor, template_number, 20);  // 4
        cursor += number_of_digits + 1;
        memcpy(cursor, "Buzz\nFizz\n", 10);  // 5, 6
        cursor += 10;
        memcpy(cursor, template_number, 20);  // 7
        cursor += number_of_digits + 1;
        memcpy(cursor, template_number, 20);  // 8
        cursor += number_of_digits + 1;
        memcpy(cursor, "Fizz\nBuzz\n", 10);  // 9, 10
        cursor += 10;
        memcpy(cursor, template_number, 20);  // 11
        cursor += number_of_digits + 1;
        memcpy(cursor, "Fizz\n", 5); // 12
        cursor += 5;
        memcpy(cursor, template_number, 20);  // 13
        cursor += number_of_digits + 1;
        memcpy(cursor, template_number, 20);  // 14
        cursor += number_of_digits + 1;
        memcpy(cursor, "FizzBuzz\n", 9);  // 15
}

static size_t fizzbuzz_memoized(uint64_t start, uint64_t stop, char *restrict buffer) {
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
    char template_number[21];
    write_number(start, number_of_digits, template_number);
    char *prefix = template_number;
    size_t prefix_length = number_of_digits - 3;
    
    /* Template the fizzes and buzzes and numbers. In the unroll loop we 
       will only replace the last digits. 
       The largest template will need will have 8 numbers with size of 20,
       5 fizzes, 3 buzzes (including fizzbuzz) and 15 newlines. This equals
       8*20 + 5*4 + 3*4 + 15 = 160 + 20 + 12 + 15 = 207. We make the template
       208 units because that is equal to 13 * 16 and the compiler can use
       16 byte vectors for copying.
       Also calculate the offset where the template numbers should be. This
       is constant for each loop.
       */
    char unroll_template[208];
    initialize_unroll_template(unroll_template, template_number, number_of_digits);
    size_t unroll_template_size = 8 * number_of_digits + 8 * 4 + 15;
    /* For each offset, number of preceding numbers times length, number
       off fizzes/buzzes times 4 and number of newline characters + 
       prefix length. */
    size_t offset1  = 0 * number_of_digits + 0 * 4 +  0 + prefix_length; 
    size_t offset2  = 1 * number_of_digits + 0 * 4 +  1 + prefix_length;
    size_t offset4  = 2 * number_of_digits + 1 * 4 +  3 + prefix_length;
    size_t offset7  = 3 * number_of_digits + 3 * 4 +  6 + prefix_length;
    size_t offset8  = 4 * number_of_digits + 3 * 4 +  7 + prefix_length;
    size_t offset11 = 5 * number_of_digits + 5 * 4 + 10 + prefix_length;
    size_t offset13 = 6 * number_of_digits + 6 * 4 + 12 + prefix_length;
    size_t offset14 = 7 * number_of_digits + 6 * 4 + 13 + prefix_length;
    size_t index = 0;
    size_t i = start;
    while (i < stop && i % 15 != 1) {
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
            buffer_size += number_from_prefix_and_index(
                prefix, prefix_length, index, buffer + buffer_size);
        }
        index += 1;
        i += 1;
    }   

    /*This is the hot loop where most of the work happens */
    uint64_t unroll_stop = stop - 14;
    for (; i<unroll_stop; i+=15 ) {
        char *restrict cursor = buffer + buffer_size;
        /* Fixed size memcpy faster than function call */
        memcpy(cursor, unroll_template, sizeof(unroll_template));
        memcpy(cursor + offset1,  zero_to_999 + ((index +  0) * 4), 4);
        memcpy(cursor + offset2,  zero_to_999 + ((index +  1) * 4), 4);
        memcpy(cursor + offset4,  zero_to_999 + ((index +  3) * 4), 4);
        memcpy(cursor + offset7,  zero_to_999 + ((index +  6) * 4), 4);
        memcpy(cursor + offset8,  zero_to_999 + ((index +  7) * 4), 4);
        memcpy(cursor + offset11, zero_to_999 + ((index + 10) * 4), 4);
        memcpy(cursor + offset13, zero_to_999 + ((index + 12) * 4), 4);
        memcpy(cursor + offset14, zero_to_999 + ((index + 13) * 4), 4);
        cursor += 15;
        index += 15;
        buffer_size += unroll_template_size;
    }
    /* End of hot loop. */

    for (; i < stop; i++) {
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
            buffer_size += number_from_prefix_and_index(
                prefix, prefix_length, index, buffer + buffer_size);
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
