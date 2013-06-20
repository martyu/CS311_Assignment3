CS311_Assignment3
=================

C++ program that finds the prime numbers in a given range using multiple parallel threads, and keeps track of them with a bitmap.


## Overview
This program finds all prime numbers between one and a number taken as a command line argument between one and 2^32.  It prints out the number of primes in the range, and prints the numbers to an output file named primes.txt.  


## Functions
The program contains one source file called "main.cpp".  This file contains four functions that each handle a separate task.  They are: main(), bit_indexes_from_number(), print_bitmap(), check_if_bit_index_is_prime().

### main()
The main() function handles the program.  It should probably have been split into more functions, but none of them seemed large enough to need their own function.  All the thread creating is done here, so the function acts as a sort of home base for the other threads.

### bit_indexes_from_number()
This function takes in an unsigned long long int and decides what the indexes of the number will be to locate the bit in the bitmap.  The first component of the two values it returns is the index of the long long that the bit is in.  The second component is the offset in that long long.

### print_bitmap()
This function prints the contents of the bitmap such that bits set to 0 are written to the output file containing all the primes.  It also prints out the the screen the number of primes in the range.

### check_if_bit_index_is_prime()
This function simply takes in the index returned from bit_indexes_from_number() and checks if the value is set to 0.


## Design
The program works by iterating through the list if numbers (represented as a bitmap) and when it finds a 0 bit, determines that number is a prime, and creates threads to mark its multiples as 1, representing non-primes.
