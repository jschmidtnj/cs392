# segmented sieve of eratosthenes

find all primes between 29 and 40

a = 29
b = 40

low_primes = [2, 3, 5]

// 40 - 29 + 1 = 12 elements
high_primes:
29 30 ... -> they are mappings to these numbers
0  1  2  3  4  5  6  7  8  9  10  11
T  T  T  T  T  T  T  T  T  T   T   T
T  F  T  F  T  F  T  F  T  F   T   F - mult. of 2
T  F  T  F  F  F  T  F  T  F   F   F - mult. of 3
T  F  T  F  F  F  F  F  T  F   F   F - mult. of 5

p = 2
29.0 / 2 = 15.0 (ceiling index)
15 * 2 - 29 = 1
multiples of 2 start at index 1

p = 3
10 * 3 - 29 = 1
multiples of 3 start at index 1

p = 5
6 * 5 - 29 = 1
multiples of 5 starts at index 1
