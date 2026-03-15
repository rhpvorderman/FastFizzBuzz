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

