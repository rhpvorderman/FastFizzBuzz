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

# Uncouple number of decimal calculation
