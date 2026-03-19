# FastFizzBuzz

This is just an exercise to attempt to write a really fast FizzBuzz implementation.
There are already answers online that do this, but it is fun to struggle and 
try it myself. Then compare other solutions later. 

I do know a few things beforehand:
1. Buffering is apparently necessary and helps a lot with speed.
2. SIMD can be used.
3. Multithreading can be used.
4. One of the solutions reached 30 GB/s on the submitters computer.

# First step: Python.

This is [very straightforward](./reference.py). It runs at roughly 27 MiB/s on 
my laptop. Not very impressive.

# Next step: C.
[Also very straightforward](./reference.c). Compile with `gcc -O3 ./reference.c` and
the result reachees 250 MiB/s. Workable, but not good enough. Now the fun can 
begin!

# Verification
The reference program should output something that md5 hashes to 
`6c272a12ed1a48b4bea560ecf642a57a`.

# Buffering

Simply [adding a buffer](./buffered.c) and printing that adds a little 
speed: 350 MiB/s.
Not stellar, but also not bad at all.

# Better number conversion.

In the buffered version `sprintf` is used for the number formatting. This 
is a very generic function. We only need to output positive, unsigned, decimal
numbers. So there is a lot of work that can be saved for writing a function
that can do just that. 

- With how ASCII works `'0' + 1 == '1'` etc. So that makes printing single 
  digits easy. 
- We can use modulo and integer division to find the digits. Since we will use
  constants for the modulo and division, the compiler will optimize these to
  bitwise opterations.
- A problem with number printing is not knowing how long the number is going 
  to be. Since we increment, we can simply track the number of digits, and
  increase these once a treshold is hit. 

By doing it fairly naively, converting one decimal at the time and making sure
 the decimals are known beforehand the code becomes 
 [a bit larger](./number_conversion.c), but now speeds of 1GiB/s are reached!

 Since we are now running in the speed limits of MD5SUM to verify we will use 
 xxh128sum instead. The number to look for is 89376fe05131799159c9907addd39b66.

# Uncouple number of decimal calculation
Since the range between 2^8 (256) and 2^9 (512) holds the same number of 
decimals we can use this fact to create a quick number of decimal calculation.
Simply use the leading zero count and we know the most significant bit. Then
we can use a lookup table in order to get the correct number of digits. In
ambiguous cases (2^9 is 512 and numbers with the 9th bit set go up to 1023) 
we check if the number exceeds the threshold. Since there is only one threshold
per ambigious case this check is quite fast. 

This is [done here](./number_of_decimal_calculation.c). The advantage of doing
this is that we can make the loop a bit simpler. The disadvantage is that
it is slightly slower (by 1-2%) but since this calculation allows is to
determine the number of digits at any point without having to track it this
confers some advantages for later.

# Uncouple buffer flush calculation

Rather than calculating the buffer usage and flushing accordingly, we can
also create a fizzbuzz function with a start and stop and a buffer argument. 
We can ensure the buffer is always big enough for the given amount of work.
This will make the loop simpler still.

This is [done here](./uncouple_buffer.c). It is slightly slower because less
of the buffer is populated and more write calls are used, but this can 
probably be optimized later. For now the fizzbuzz function is simple again
and we can look to further optimizations.

# Memoization

Since we now process ranges of numbers in the fizzbuzz function, the ranges
can be strategically chosen. The range 1000-1999 for instance never changes
the number of digits. This is also true for other ranges of thousand such 
as 357000-357999 and 12412434000-12412434999. What also is true is that in 
a range of thousand, all numbers preceding the last three digits remain the 
same. 

So we can:

+ Only calculate the number of digit once per thousand.
+ Calculate the prefix once per thousand.
+ Only calculate the last three digits, but rather than doing this, a 1000-entry
  lookup table is more efficient.

This is done in [memoization.c](./memoization.c) which causes a significant
speedup. 

# Loop unrolling. 

Fizzbuzz is a repeating process. 
1. 1,2, fizz, 4, buzz, fizz, 7, 8, fizz, buzz, 11, fizz, 13, 14, fizzbuzz
2. 16, 17, fizz, 19, buzz, fizz, 22, 23, fizz, buzz,  26, fizz, 28, 29, fizzbuzz
The fizzes and buzzes are always in the same place. So we can do some loop
unrolling and remove the nasty tests that mess with the branch prediction as
no tests are needed.

What is done to achieve high speed is to take the first number of the thousand.
Then fizzbuzz is entirely written out with 15 numbers, with the first number as a template.
Then on each unrolled loop the last digits for each number in the template are updated to
the correct ones. This requires one memory copy of the template and then 8 additional DWORD
copies in the correct places. This is very fast.

The result is in [loop_unrolling.c](./loop_unrolling.c) and this reaches 
speeds of 5 GB/s. Not too shabby. 

# Larger buffers

Given the large amount of system time, it might be that simply too many write calls are 
issued. Every group of thousand is directly written. Instead, we now use a 256K buffer.
This fits in L2 cache (512K in this machine.) The size of a loop is estimated and if it
would overflow the buffer, the buffer is written to stdout.
Larger sizes do not confer extra advantages.

The result is in [larger_buffer.c](./larger_buffer.c). This runs the entire 
program in 1.8 seconds. That equates to more than 16 GB/s. Quite great. 
That is better than the best single-threaded result I saw online when I did
a quick scan before I started this challenge. But still, there could be 
much better implementations out there.

# Going up to ten thousand.

With the larger buffers, the code is very fast, but we only spend 66 iterations
in the fast loop. So we increase the chunks 
[from 1000 to 10000](./ten_thousand.c). Now we spend around 666 iterations in 
the fast loop. Interestingly, this does not do much. It reduces runtime from
1.8 to 1.7 seconds. Still a meaningful percentage, but not that much faster.
 On the other hand, since so much time is now spent in the loop rather than 
 outside it maybe we can do some...

# Simplification

- First break out the little alignment loops at the beginning and end of
  the unroll function. These can be fulfilled by the same function.
- Remove the templating function as this can also be performed by simply 
  running fizzbuzz.
- Since we only create a template number once every ten thousand times, 
  `sprintf` can be used instead. This hardly slows down the program and 
  the write_number function can be removed. It also calculates the number
  of digits automatically.
- Since the number of digits is only needed once outside of the memoized 
  function to precalculate the buffer we can simplify that calculation. It 
  is okay if it uses slightly more cycles.
 
 [These changes](./simplification.c) saved 120 lines of code out of 360. 
 Not too shabby. Let us look for further optimizations.

 # Further optimization opportunities.

 Most of the work happens in the following loop:

 ```C
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
 ```
 Size of unroll template is 208 bytes. The maximum length of fizzbuzz given 
 a 64 bit unsigned integer is 8 numbers of length 20, 4 times fuzz, 2 times buzz
 one time fizzbuzz and 15 newlines. That adds up to 207 characters. 208 is 
 used because that is a multiple of 16. 
 
 So first a template is pasted in the buffer. Then the number positions are 
 updated. 

 Let's look at the assembly:
 ```assembly
.L34:
        mov     r15d, DWORD PTR zero_to_9999[0+rbp*4]
        movups  XMMWORD PTR [rbx], xmm12
        add     r12, 15
        mov     r14, rbx
        movups  XMMWORD PTR [rbx+16], xmm11
        movups  XMMWORD PTR [rbx+32], xmm10
        movups  XMMWORD PTR [rbx+48], xmm9
        movups  XMMWORD PTR [rbx+64], xmm8
        movups  XMMWORD PTR [rbx+80], xmm7
        movups  XMMWORD PTR [rbx+96], xmm6
        movups  XMMWORD PTR [rbx+112], xmm5
        movups  XMMWORD PTR [rbx+128], xmm4
        movups  XMMWORD PTR [rbx+144], xmm3
        movups  XMMWORD PTR [rbx+160], xmm2
        movups  XMMWORD PTR [rbx+176], xmm1
        movups  XMMWORD PTR [rbx+192], xmm0
        mov     DWORD PTR [rbx-4+r13], r15d
        mov     r15d, DWORD PTR zero_to_9999[4+rbp*4]
        mov     DWORD PTR [rbx+r11], r15d
        mov     r15d, DWORD PTR zero_to_9999[12+rbp*4]
        mov     DWORD PTR [rbx+7+r8], r15d
        mov     r15d, DWORD PTR zero_to_9999[24+rbp*4]
        mov     DWORD PTR [rbx+18+rdi], r15d
        mov     r15d, DWORD PTR zero_to_9999[28+rbp*4]
        mov     DWORD PTR [rbx+19+rsi], r15d
        mov     r15d, DWORD PTR zero_to_9999[40+rbp*4]
        mov     DWORD PTR [rbx+30+rcx], r15d
        mov     r15d, DWORD PTR zero_to_9999[48+rbp*4]
        mov     DWORD PTR [rbx+36+rdx], r15d
        mov     r15d, DWORD PTR zero_to_9999[52+rbp*4]
        add     rbp, 15
        mov     DWORD PTR [rbx+37+r10], r15d
        add     rbx, rax
        cmp     r12, r9
        jb      .L34
 ```
So as stated in the comment, memcpy with a fixed constant is optimized. In fact,
GCC has saved all of the template in xmm0 up to xmm12. 13 registers of each
16 bytes (13 * 16 = 208). Since the vector registers are not used elsewhere it
can simply store the data in there and only perform a write operation on each
loop, without having to perform a read operation. Very efficient!

The following code calculates an addres in the r15d register and moves a 
value in there that is captured by a lookup. 

I am a bit annoyed by the templating always overshooting, but that is needed
to support the longest integers and just 13 movups instructions is not going 
to be more costly than a memcpy call with an arbitrary length. 
memcpy with a very long length can be very efficient though, as it can use
avx2 vectors which are present on this machine. However filling up the buffer
that way and then writing on it all over again does not feel like it will
provide a great benefit because of cache eviction.
