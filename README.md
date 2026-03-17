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
  the unroll function. These van be fulfilled by the same function.