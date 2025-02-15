# Prime-Generator
A prime number generator that writes primes to a .bin file.

# Contents
This repository contains two C++ files:

`prime_generator.cpp` – The actual prime generator.
`prime_loader.cpp` – A loader used to open the .bin file and print its contents.
This generator has been developed with a focus on performance, even for large numbers. It implements several optimizations to improve execution time and reduce memory usage.

# Optimizations:
Segmented prime generation/storage – Reduces memory usage.
Multithreading – Improves generation speed (achieved via OpenMP).

# Compilation
To compile the program for the best performance, run:

`g++ prime_generator.cpp -o prime_generator -O3 -fopenmp`
And for the loader:
`g++ prime_loader.cpp -o prime_loader -O3`
