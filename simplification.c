#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE (256 * 1024)

static char zero_to_9999[40000];

static void initialize_zero_to_9999(void) {
    for (size_t i=0; i<10; i++) {
        for (size_t j=0; j<10; j++) {
            for (size_t k=0; k<10; k++) {
                for (size_t l=0; l<10; l++) {
                    size_t pos = (i * 1000 + j * 100 + k * 10 + l * 1) * 4;
                    zero_to_9999[pos + 0] = i + '0';
                    zero_to_9999[pos + 1] = j + '0';
                    zero_to_9999[pos + 2] = k + '0';
                    zero_to_9999[pos + 3] = l + '0';
                }
            }
        }
    }
}

static inline uint64_t uint64_min(uint64_t a, uint64_t b) {
    if (a < b) {
        return a;
    }
    return b;
}

static inline size_t calculate_number_of_digits(uint64_t number) {
    size_t number_of_digits = 0;
    while (number) {
        number = number / 10;
        number_of_digits += 1;
    }
    return number_of_digits;
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
            buffer_size += sprintf(buffer+buffer_size, "%lu\n", i);
        }
    }   
    return buffer_size;
}

static size_t fizzbuzz_memoized(
    uint64_t start, 
    uint64_t stop, 
    size_t index, 
    char *prefix,
    size_t prefix_length,
    char *restrict buffer
)
{
    size_t buffer_size = 0;
    for (size_t i=start; i < stop; i++) {
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
           /* Using a fixed memcpy size avoids a function call. The largest 
                integer is 20 digits in size.*/
            memcpy(buffer + buffer_size, prefix, 20);
            buffer_size += prefix_length;
            /* Lookup the last 3 digits. */
            size_t pos = index * 4;
            memcpy(buffer + buffer_size , zero_to_9999 + pos, 4);
            buffer[buffer_size + 4] = '\n';
            buffer_size += 5;    
        }
        index += 1;
    }
    return buffer_size;

}

static size_t fizzbuzz_memoized_unrolled(uint64_t start, uint64_t stop, char *restrict buffer) {
    if (start % 10000 != 0 || stop - start > 10000) {
        fprintf(
            stderr, 
            "Assumption for memoization broken, start must be divisible by "
            "10000, stop must be at most 10000 positions away from start.\n"
            "Start: %lu; Stop: %lu\n", start, stop
        );
        exit(1);
    }
    char template_number[21];
    size_t number_of_digits = sprintf(template_number, "%lu", start);
    char *prefix = template_number;
    size_t prefix_length = number_of_digits - 4;

    /* First we roll up to the point where we can start unrolling the loop. 
    */
    static uint8_t iterations[15] = {1, 0, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 
        4, 3, 2};
    size_t align_count = iterations[start % 15];
    size_t align_stop = uint64_min(start + align_count, stop);

    size_t buffer_size = fizzbuzz_memoized(
        start, align_stop, 0, prefix, prefix_length, buffer);;
    size_t index = align_count;

    /* The largest template will need will have 8 numbers with size of 20,
       5 fizzes, 3 buzzes (including fizzbuzz) and 15 newlines. This equals
       8*20 + 5*4 + 3*4 + 15 = 160 + 20 + 12 + 15 = 207. We make the template
       208 units because that is equal to 13 * 16 and the compiler can use
       16 byte vectors for copying.
       Also calculate the offset where the template numbers should be. This
       is constant for each loop.
       */
    char unroll_template[208];
    fizzbuzz_memoized(align_stop, align_stop + 16, index, prefix, 
        prefix_length, unroll_template);
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
    
 
    size_t i = align_stop;
    /*This is the hot loop where most of the work happens */
    uint64_t unroll_stop = stop - 14;
    for (; i<unroll_stop; i+=15 ) {
        char *restrict cursor = buffer + buffer_size;
        /* Fixed size memcpy faster than function call */
        memcpy(cursor, unroll_template, sizeof(unroll_template));
        memcpy(cursor + offset1,  zero_to_9999 + ((index +  0) * 4), 4);
        memcpy(cursor + offset2,  zero_to_9999 + ((index +  1) * 4), 4);
        memcpy(cursor + offset4,  zero_to_9999 + ((index +  3) * 4), 4);
        memcpy(cursor + offset7,  zero_to_9999 + ((index +  6) * 4), 4);
        memcpy(cursor + offset8,  zero_to_9999 + ((index +  7) * 4), 4);
        memcpy(cursor + offset11, zero_to_9999 + ((index + 10) * 4), 4);
        memcpy(cursor + offset13, zero_to_9999 + ((index + 12) * 4), 4);
        memcpy(cursor + offset14, zero_to_9999 + ((index + 13) * 4), 4);
        cursor += 15;
        index += 15;
        buffer_size += unroll_template_size;
    }
    /* End of hot loop. */
    buffer_size += fizzbuzz_memoized(
        i, stop, index, prefix, prefix_length, buffer + buffer_size);
    return buffer_size;
}

int main() {
    char buffer[BUFFER_SIZE];
    initialize_zero_to_9999();
    uint64_t start = 1;
    uint64_t end = 1ULL << 32ULL;

    /* First calculate the result up to 10000 */
    uint64_t stop = uint64_min(10000, end);
    size_t buffer_size = fizzbuzz(start, stop, buffer);
    fwrite(buffer, 1, buffer_size, stdout);
    start = 10000;
    if (end <= 10000) {
        fflush(stdout);
        return 0;
    }
    buffer_size = 0;
    while (start < end) {
        size_t number_of_digits = calculate_number_of_digits(start);
        /* Estimate the output volume on the buffer. Every 15 units we make 
           a full round. That is 8 fizzes/buzzes. 8 numbers and 15 newlines.
           There are 667 of these rounds in a 10000. We add 208 because that is the size
           of a template. */
        size_t estimated_output_size = 667 * (8 * 4 + 8 * number_of_digits + 15) + 208;
        if (buffer_size + estimated_output_size > BUFFER_SIZE) {
            fwrite(buffer, 1, buffer_size, stdout);
            buffer_size = 0;
        }
        stop = uint64_min(start + 10000, end);
        buffer_size += fizzbuzz_memoized_unrolled(start, stop, buffer + buffer_size);
        start = stop;
    }
    fwrite(buffer, 1, buffer_size, stdout);
    fflush(stdout);
    return 0;
}
