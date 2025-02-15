#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <omp.h>
#include <cstring>

// Timing and time formating
#include <chrono>
#include <iomanip>

// Define the Number type based on the platform.
typedef int64_t Number;

// block size, adjust based on available memory and cache
const int sliceSize = 128 * 1024;

// Function to write primes in binary format incrementally (appending to the file)
void WritePrimesBinary(const std::vector<Number>& primes, const std::string& filename) {
    std::ofstream outFile(filename, std::ios::binary | std::ios::app);  // Open file in binary mode and append
    if (!outFile) {
        std::cerr << "Error opening the file for writing!" << std::endl;
        return;
    }

    // Write primes as raw 64-bit integers
    for (Number prime : primes) {
        outFile.write(reinterpret_cast<const char*>(&prime), sizeof(prime));
    }

    outFile.close();
}

// Function to find primes up to 'sqrtLast' using the normal sieve of Eratosthenes
void simpleSieve(Number limit, std::vector<Number>& primes) {
    bool* isPrime = new bool[limit + 1];
    std::fill(isPrime, isPrime + limit + 1, true);
    isPrime[0] = isPrime[1] = false;  // 0 and 1 are not primes

    for (Number p = 2; p * p <= limit; p++) {
        if (isPrime[p]) {
            for (Number i = p * p; i <= limit; i += p) {
                isPrime[i] = false;
            }
        }
    }

    // Collecting primes
    for (Number p = 2; p <= limit; p++) {
        if (isPrime[p]) {
            primes.push_back(p);
        }
    }

    delete[] isPrime;
}

// Segmented Sieve Function: Finds primes up to 'lastNumber' in segments and appends to a file
int segmentedSieve(Number lastNumber, bool useOpenMP, const std::string& filename) {
    Number sqrtLast = static_cast<Number>(sqrt(static_cast<double>(lastNumber)));

    // Step 1: Find all primes up to sqrt(lastNumber) using simple sieve
    std::vector<Number> basePrimes;
    simpleSieve(sqrtLast, basePrimes);

    // Step 2: Segmented sieve over the range [2, lastNumber]
    int found = 0;
    omp_set_num_threads(useOpenMP ? omp_get_num_procs() : 1);

    // Vector to store primes from all threads before writing to file
    std::vector<Number> allPrimes;

    // Parallelize over chunks
    // Parallel loop to process each chunk.
    #pragma omp parallel for reduction(+:found)
    for (Number low = 2; low <= lastNumber; low += sliceSize) {
        Number high = std::min(low + sliceSize - 1, lastNumber);
        int segmentSize = high - low + 1;

        // Initialize sieve for this chunk.
        Number memorySize = (segmentSize) / 2 + 1;  // Only track odd numbers
        char* isPrime = new char[memorySize];
        std::fill(isPrime, isPrime + memorySize, 1);  // Initially mark all as prime

        // Step 3: Parallel sieve marking for multiples of base primes.
		// This causes a thread race. DO NOT UNCOMMENT!
		//#pragma omp parallel for
        for (Number i = 0; i < basePrimes.size(); i++) {
            Number prime = basePrimes[i];
            if (prime * prime > high) continue;

            // Find where to start marking multiples of prime in the range [low, high]
            Number start = std::max(prime * prime, (low + prime - 1) / prime * prime);
            if (start > high) continue;

            // Ensure we start marking from odd multiples
            if (start % 2 == 0) start += prime;  // Skip even multiples

            for (Number j = start; j <= high; j += 2 * prime) {
                if (j % 2 != 0) {  // Only mark odd numbers
                    isPrime[(j - low) / 2] = 0;  // Mark as non-prime
                }
            }
        }

        // Step 4: Collect primes from the segment.
        std::vector<Number> primesInSegment;
        for (Number i = 0; i < segmentSize; i++) {
            if (i == 0 && low == 2) {
                primesInSegment.push_back(2);  // Special case for 2
            } else if (i % 2 != 0 && isPrime[i / 2] == 1) {
                primesInSegment.push_back(low + i);
                found++;  // Increment prime count
            }
        }

		#pragma omp critical
		{
			WritePrimesBinary(primesInSegment, filename);
		}

        delete[] isPrime;
    }

    return found;
}

// Driver Code
int main() {
    Number lastNumber;

    // Prompt user for input
    std::cout << "Enter the maximum number to find primes up to: ";
    std::cin >> lastNumber;

    printf("Primes between 2 and %lld\n\n", lastNumber);
    printf("OpenMP uses up to %d threads running on %d processors\n\n", omp_get_max_threads(), omp_get_num_procs());

    std::string filename = "primos_seg4.bin";
    
    // Remove the file if it exists, so we can write a fresh file
    std::remove(filename.c_str());

    auto start = std::chrono::high_resolution_clock::now();
    int found = segmentedSieve(lastNumber, true, filename);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    printf("%d primes found in %.3fs\n\n", found, duration);

    return 0;
}
