#ifndef _ComputePrimes_h_
#define _ComputePrimes_h_

#include <math.h>


// The most straightforward algorithm to calculate primes.
// It fills a_storedPrimes with primes until the array is full.
void ComputePrimesSimple(int* a_storedPrimes, int a_storedPrimesSize)
{
    int numPrimes = 0;

    // Test all numbers to see if they are divisible by any previous number.
    for(int i = 2; ; ++i)
    {
        int j;
        for (j = 2; j < i; ++j)
        {
            if (i % j == 0)
            {
                break;
            }
        }

        if (i == j)
        {
            // We have found a prime!
            a_storedPrimes[numPrimes++] = i;
            if(numPrimes == a_storedPrimesSize)
            {
                return;
            }
        }
    }
}


// An algorithm with some optimizations to find primes
// Fills a_storedPrimes with primes up to a_max or until the array is full.
void ComputePrimesFast(int* a_storedPrimes, int a_storedPrimesSize)
{
    // We know 2 is a prime, so we store this so the algorithm can start on an odd number.
    int numPrimes = 1;
    a_storedPrimes[0] = 2;
    int j;

    // Start from an odd number and increment by 2, thus skipping all the even numbers which can't be primes.
    for(int i = 3; ; i+=2)
    {
        // Calculate the square root of the possible prime. We only need to consider divisors <= to this.
        int maxDivisor = (int)sqrt((double)i);

        // Iterate through the list of previous primes rather than all possible divisors.
        for (j = 1; j < numPrimes; ++j)
        {
            if(a_storedPrimes[j] > maxDivisor)
            {
                j = numPrimes;
                break;
            }

            // Test the fail case for the number being a prime
            if (i % a_storedPrimes[j] == 0)
            {
                break;
            }
        }

        if (numPrimes == j)
        {
            // We found a prime!
            a_storedPrimes[numPrimes++] = i;
            if(numPrimes == a_storedPrimesSize)
            {
                return;
            }
        }
    }
}

#endif
