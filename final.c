#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define FIZZ "Fizz\n"
#define FIZZ_SIZE 5
#define BUZZ "Buzz\n"
#define BUZZ_SIZE 5
#define FIZZBUZZ "FizzBuzz\n"
#define FIZZBUZZ_SIZE 9
/* The largest integer (UINT64_MAX) is 20 decimal digits in size.*/
#define MAX_NUMBER_OF_DIGITS 20
#define FIZZBUZZ_ROUND_SIZE (FIZZ_SIZE * 4 + BUZZ_SIZE * 2 + FIZZBUZZ_SIZE + (MAX_NUMBER_OF_DIGITS + 1) * 8)
/* We do 2000 rounds of fizzbuzz in a buffer. With a digit length of 20 at 
uint64 max we need 207 characters per round. Add a 1000 characters for 
overshoot. */
#define BUFFER_SIZE (FIZZBUZZ_ROUND_SIZE * 2000 + 1000)

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
            memcpy(buffer + buffer_size, FIZZBUZZ, FIZZBUZZ_SIZE);
            buffer_size += FIZZBUZZ_SIZE;
        }
        else if (i % 3 == 0) {
            memcpy(buffer + buffer_size, FIZZ, FIZZ_SIZE);
            buffer_size += FIZZ_SIZE;
        }
        else if (i % 5 == 0) {
            memcpy(buffer + buffer_size, BUZZ, BUZZ_SIZE);
            buffer_size += BUZZ_SIZE;
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
            memcpy(buffer + buffer_size, FIZZBUZZ, FIZZBUZZ_SIZE);
            buffer_size += FIZZBUZZ_SIZE;
        }
        else if (i % 3 == 0) {
            memcpy(buffer + buffer_size, FIZZ, FIZZ_SIZE);
            buffer_size += FIZZ_SIZE;
        }
        else if (i % 5 == 0) {
            memcpy(buffer + buffer_size, BUZZ, BUZZ_SIZE);
            buffer_size += BUZZ_SIZE;
        }
        else {
           /* Using a fixed memcpy size avoids a function call. */
            memcpy(buffer + buffer_size, prefix, MAX_NUMBER_OF_DIGITS);
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
    char template_number[MAX_NUMBER_OF_DIGITS + 1];  // +1 for terminating null.
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
    size_t offset1  = 0 * (number_of_digits + 1) + 0 * FIZZ_SIZE + 0 * BUZZ_SIZE + prefix_length; 
    size_t offset2  = 1 * (number_of_digits + 1) + 0 * FIZZ_SIZE + 0 * BUZZ_SIZE + prefix_length;
    size_t offset4  = 2 * (number_of_digits + 1) + 1 * FIZZ_SIZE + 0 * BUZZ_SIZE + prefix_length;
    size_t offset7  = 3 * (number_of_digits + 1) + 2 * FIZZ_SIZE + 1 * BUZZ_SIZE + prefix_length;
    size_t offset8  = 4 * (number_of_digits + 1) + 2 * FIZZ_SIZE + 1 * BUZZ_SIZE + prefix_length;
    size_t offset11 = 5 * (number_of_digits + 1) + 3 * FIZZ_SIZE + 2 * BUZZ_SIZE + prefix_length;
    size_t offset13 = 6 * (number_of_digits + 1) + 4 * FIZZ_SIZE + 2 * BUZZ_SIZE + prefix_length;
    size_t offset14 = 7 * (number_of_digits + 1) + 4 * FIZZ_SIZE + 2 * BUZZ_SIZE + prefix_length;
 
    size_t i = align_stop;
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
    buffer_size += fizzbuzz_memoized(
        i, stop, index, prefix, prefix_length, buffer + buffer_size);
    return buffer_size;
}

typedef struct _FizzBuzzTemplate {
    uint64_t templated_from;
    size_t template_size;
    uint8_t number_of_digits;
    // The offset to the point where the fizzbuzz cycle starts and the loop can be unrolled.
    uint8_t number_of_pre_unroll_offsets;  
    uint8_t pre_unroll_offsets[5];  
    uint8_t offset_to_unroll;  
    uint8_t unroll_length;
    uint8_t unroll_offsets[8];
    char *buffer;
} FizzBuzzTemplate;

static size_t FizzBuzzTemplate_initialize_buffer(
    FizzBuzzTemplate *fizzbuzz_template, uint64_t start, char *buffer) 
{
    if (start % 10000 != 0) {
        fprintf(
            stderr,
            "Assumption for template initialization broken, start must" 
            "be divisible by 10000. Start: %lu", start
        );
        exit(1);
    }
    size_t number_of_digits = calculate_number_of_digits(start);
    switch (start % 15) {
        case 0:
            // print 'fizzbuzz\n' and then immediately start; 
            fizzbuzz_template->offset_to_unroll = 9;  // Length of fizzbuzz one newline.
            fizzbuzz_template->number_of_pre_unroll_offsets = 0;
            break;
        case 5:
            // buzz, fizz, 7, 8, fizz, buzz, 11, fizz, 13, 14, fizzbuzz
            fizzbuzz_template->number_of_pre_unroll_offsets = 5;
            fizzbuzz_template->pre_unroll_offsets[0] = 1 * FIZZ_SIZE + 1 * BUZZ_SIZE + 0 * (number_of_digits + 1); // 7 
            fizzbuzz_template->pre_unroll_offsets[1] = 1 * FIZZ_SIZE + 1 * BUZZ_SIZE + 1 * (number_of_digits + 1); // 8
            fizzbuzz_template->pre_unroll_offsets[2] = 2 * FIZZ_SIZE + 2 * BUZZ_SIZE + 2 * (number_of_digits + 1); // 11
            fizzbuzz_template->pre_unroll_offsets[3] = 3 * FIZZ_SIZE + 2 * BUZZ_SIZE + 3 * (number_of_digits + 1); // 13
            fizzbuzz_template->pre_unroll_offsets[4] = 3 * FIZZ_SIZE + 2 * BUZZ_SIZE + 4 * (number_of_digits + 1); // 14
            fizzbuzz_template->offset_to_unroll = 3 * FIZZ_SIZE + 2 * BUZZ_SIZE + 5 * (number_of_digits + 1) + FIZZBUZZ_SIZE; // FizzBuzz
            break;
        case 10:
            // buzz, 11, fizz, 13, 14, fizzbuzz
            fizzbuzz_template->number_of_pre_unroll_offsets = 3;
            fizzbuzz_template->pre_unroll_offsets[0] = 0 * FIZZ_SIZE + 1 * BUZZ_SIZE + 0 * (number_of_digits + 1); // 11
            fizzbuzz_template->pre_unroll_offsets[1] = 1 * FIZZ_SIZE + 1 * BUZZ_SIZE + 1 * (number_of_digits + 1); // 13
            fizzbuzz_template->pre_unroll_offsets[2] = 1 * FIZZ_SIZE + 1 * BUZZ_SIZE + 2 * (number_of_digits + 1); // 14
            fizzbuzz_template->offset_to_unroll = 1 * FIZZ_SIZE + 1 * BUZZ_SIZE + 3 * (number_of_digits + 1) + FIZZBUZZ_SIZE; // FizzBuzz
            break;
        default:
            fprintf(
                stderr, 
                "Faulty initialization with start: %lu, %lu %% 15 = %lu\n", 
                start, start, start % 15
            );
            exit(1);
    }
    fizzbuzz_template->number_of_digits = number_of_digits;
    fizzbuzz_template->templated_from = start;
    /* For each offset, number of preceding numbers times length, number
       off fizzes/buzzes times 5 */
    /* 1, 2, fizz, 4, buzz, fizz, 7, 8, fizz, buzz, 11, fizz, 13, 14, fizzbuzz */
    fizzbuzz_template->unroll_offsets[0] = 0 * (number_of_digits + 1) + 0 * FIZZ_SIZE + 0 * BUZZ_SIZE; // 1
    fizzbuzz_template->unroll_offsets[1] = 1 * (number_of_digits + 1) + 0 * FIZZ_SIZE + 0 * BUZZ_SIZE; // 2
    fizzbuzz_template->unroll_offsets[2] = 2 * (number_of_digits + 1) + 1 * FIZZ_SIZE + 0 * BUZZ_SIZE; // 4
    fizzbuzz_template->unroll_offsets[3] = 3 * (number_of_digits + 1) + 2 * FIZZ_SIZE + 1 * BUZZ_SIZE; // 7
    fizzbuzz_template->unroll_offsets[4] = 4 * (number_of_digits + 1) + 2 * FIZZ_SIZE + 1 * BUZZ_SIZE; // 8
    fizzbuzz_template->unroll_offsets[5] = 5 * (number_of_digits + 1) + 3 * FIZZ_SIZE + 2 * BUZZ_SIZE; // 11
    fizzbuzz_template->unroll_offsets[6] = 6 * (number_of_digits + 1) + 4 * FIZZ_SIZE + 2 * BUZZ_SIZE; // 13
    fizzbuzz_template->unroll_offsets[7] = 7 * (number_of_digits + 1) + 4 * FIZZ_SIZE + 2 * BUZZ_SIZE; // 14
    fizzbuzz_template->unroll_length = 8 * (number_of_digits + 1) + 4 * FIZZ_SIZE + 2 * BUZZ_SIZE + FIZZBUZZ_SIZE; 
    fizzbuzz_template->buffer = buffer;
    fizzbuzz_template->template_size = fizzbuzz_memoized_unrolled(
        start, start + 10000, fizzbuzz_template->buffer);
    return fizzbuzz_template->template_size;
}

static size_t FizzBuzzTemplate_replace_by4(
    FizzBuzzTemplate *restrict fizzbuzz_template,
    uint64_t start
)
{   
    /* Then calculate the replacement digit. */
    uint64_t replace_digits = (start / 10000) % 10000;
    char *restrict buffer = fizzbuzz_template->buffer;
    char replace_chars[4];
    size_t number_of_digits = fizzbuzz_template->number_of_digits;
    memcpy(replace_chars, zero_to_9999 + (replace_digits * 4), 4);
    // 4+4 = 8. 4 because 0000-9999 has been templated and 4 because we use a 32-bit integer.
    size_t prefix_length = number_of_digits - 8;  
    
    /* Roll up to the point beyond the first FizzBuzz */
    for (size_t i=0; i < fizzbuzz_template->number_of_pre_unroll_offsets; i++) {
        size_t offset = fizzbuzz_template->pre_unroll_offsets[i] + prefix_length;
        memcpy(buffer + offset, replace_chars, 4);
    }

    /* Template everything else. We do 667 loops here so there is overshoot. 
       This makes the logic a bit simpler and the compiler can unroll a bit 
       since the number of iterations is known.
    */
    buffer += fizzbuzz_template->offset_to_unroll;
    size_t offset0 = fizzbuzz_template->unroll_offsets[0] + prefix_length;
    size_t offset1 = fizzbuzz_template->unroll_offsets[1] + prefix_length;
    size_t offset2 = fizzbuzz_template->unroll_offsets[2] + prefix_length;
    size_t offset3 = fizzbuzz_template->unroll_offsets[3] + prefix_length;
    size_t offset4 = fizzbuzz_template->unroll_offsets[4] + prefix_length;
    size_t offset5 = fizzbuzz_template->unroll_offsets[5] + prefix_length;
    size_t offset6 = fizzbuzz_template->unroll_offsets[6] + prefix_length;
    size_t offset7 = fizzbuzz_template->unroll_offsets[7] + prefix_length;
    size_t unroll_length = fizzbuzz_template->unroll_length;
    for(size_t i=0; i<667; i++) {
        char *restrict cursor = buffer + i * unroll_length;
        memcpy(cursor + offset0, replace_chars, 4);
        memcpy(cursor + offset1, replace_chars, 4);
        memcpy(cursor + offset2, replace_chars, 4);
        memcpy(cursor + offset3, replace_chars, 4);
        memcpy(cursor + offset4, replace_chars, 4);
        memcpy(cursor + offset5, replace_chars, 4);
        memcpy(cursor + offset6, replace_chars, 4);
        memcpy(cursor + offset7, replace_chars, 4);
    }
    return fizzbuzz_template->template_size;
}

static size_t fizzbuzz_buffer_initialize(
    uint64_t start,
    FizzBuzzTemplate *fizzbuzz_templates,
    char *restrict buffer
) 
{
    size_t buffer_size = FizzBuzzTemplate_initialize_buffer(
        fizzbuzz_templates + ((start % 15) / 5), start, buffer);
    start += 10000;
    buffer_size += FizzBuzzTemplate_initialize_buffer(
        fizzbuzz_templates + ((start % 15) / 5), start, buffer + buffer_size);
    start += 10000;
    buffer_size += FizzBuzzTemplate_initialize_buffer(
        fizzbuzz_templates + ((start % 15) / 5), start, buffer + buffer_size);
    return buffer_size;
}

static size_t fizzbuzz_templated_10000(
    uint64_t start, 
    FizzBuzzTemplate *fizzbuzz_templates,
    char *restrict buffer) {
    if (start < 10000000 || start % 10000 != 0) {
        fprintf(
            stderr, 
            "Assumption for templating broken, start must be greater "
            "than 10 000 000, divisible by "
            "10 000\n"
            "Start: %lu; ", start
        );
        exit(1);
    }
    size_t template_index = start % 15; 
    if (template_index != 0 && template_index != 5 && template_index != 10) {
        fprintf(stderr, "invalid template index: %lu, start: %lu", template_index, start);
    }
    template_index /= 5;
    FizzBuzzTemplate *template = fizzbuzz_templates + template_index;
    /* We template out four digits before the last four digits. */
    if (start / 10000000 != template->templated_from / 10000000 || 
        template->templated_from == 0) {
        return 0;
    }
    return FizzBuzzTemplate_replace_by4(template, start);
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
    uint64_t end2 = uint64_min(10000000, end);
    while (start < end2) {
        size_t number_of_digits = calculate_number_of_digits(start);
        /* Estimate the output volume on the buffer. Every 15 units we make 
           a full round..
           There are 667 of these rounds in a 10000. We add 208 because that is the size
           of a template. */
        size_t estimated_output_size = 667 * (FIZZ_SIZE * 4 + BUZZ_SIZE * 2 +  8 * (number_of_digits + 1) + FIZZBUZZ_SIZE) + 208;
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
    buffer_size = 0;
    FizzBuzzTemplate fizzbuzz_templates[3];
    memset(fizzbuzz_templates, 0, sizeof(FizzBuzzTemplate) * 3);
    while (start < end - 30000) {
        for (size_t i=0; i<3; i++) {
            size_t answer = fizzbuzz_templated_10000(
                start, fizzbuzz_templates, buffer + buffer_size);
            if (answer == 0) {
                fwrite(buffer, 1, buffer_size, stdout);
                fizzbuzz_buffer_initialize(start, fizzbuzz_templates, buffer);
                buffer_size = 0;
                break;
            }
            buffer_size += answer;
            start += 10000;
        } 
        fwrite(buffer, 1, buffer_size, stdout);
        buffer_size = 0;
    }
    fwrite(buffer, 1, buffer_size, stdout);
    fflush(stdout);
    buffer_size = fizzbuzz(start, end, buffer);
    fwrite(buffer, 1, buffer_size, stdout);
    fflush(stdout);
    return 0;
}
