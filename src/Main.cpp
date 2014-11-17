// Algorithms.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <memory>
#include <stdlib.h>

#ifdef _DEBUG
#include <vld.h>
#endif

#include "ComputePrimes.h"
#include "MergeSort.h"
#include "Timer.h"
#include "JobScheduler.h"
#include "GetMostCommonLetter.h"
#include "ReverseWords.h"
#include "Cache.h"


// function to verify the contents of an array are sorted in ascending order
bool VerifyOrder(int* a_testData, int a_dataLength)
{
    for(int i = 0; i < a_dataLength - 1; ++i)
    {
        if(a_testData[i] > a_testData[i + 1])
        {
            return false;
            break;
        }
    }
    return true;
}


int _tmain(int argc, _TCHAR* argv[])
{
    Timer timer;
    float ms;
    printf("Starting tests...\n");

    // PRIME NUMBERS
    {
        int maxPrimes = 5000;
        std::unique_ptr<int[]> primes(new int[maxPrimes]);

        timer.Reset();
        ComputePrimesSimple(primes.get(), maxPrimes);
        ms = 1000.0f * timer.Time();
        printf("Computed %d primes Simply in %f ms\n", maxPrimes, ms);

        timer.Reset();
        ComputePrimesFast(primes.get(), maxPrimes);
        ms = 1000.0f * timer.Time();
        printf("Computed %d primes Fast in %f ms\n", maxPrimes, ms);
    }

    // SORTING
    {
        // Prepare data to sort. We use the same input data for each sort test.
        const int dataLength = 2000001;
        std::unique_ptr<int[]> originalData(new int[dataLength]);
        std::unique_ptr<int[]> testData(new int[dataLength]);
        for(int i = 0; i < dataLength; ++i)
        {
            originalData[i] = rand();
        }

        MergeSort<int> mergeSorter(dataLength);

        // Test MergeSort::SortMT
        {
            memcpy(testData.get(), originalData.get(), sizeof(int) * dataLength);

            // Create queue and threads for workers (we wil use our main thread too, so create one fewer worker).
            // This is scoped so that threads cease when test is over.
            const size_t NUM_THREADS_FOR_SORTING = 8;
            const int MAX_JOBS_IN_QUEUE = 16;
            JobScheduler jobScheduler(NUM_THREADS_FOR_SORTING - 1, MAX_JOBS_IN_QUEUE);

            timer.Reset();
            mergeSorter.SortMT(testData.get(), dataLength, jobScheduler);
            ms = 1000.0f * timer.Time();
            printf("SortMT time (%d threads) %f ms\n", NUM_THREADS_FOR_SORTING, ms);
            bool success = VerifyOrder(testData.get(), dataLength);
            printf("SortMT %s\n", (success ? "success" : "FAIL"));
        }

        // Test MergeSort::SortUnrolledMemcpy
        memcpy(testData.get(), originalData.get(), sizeof(int) * dataLength);
        timer.Reset();
		mergeSorter.SortUnrolledMemcpy(testData.get(), dataLength);
        ms = 1000.0f * timer.Time();
        printf("SortUnrolledMemcpy time %f ms\n", ms);
		bool success = VerifyOrder(testData.get(), dataLength);
        printf("SortUnrolledMemcpy %s\n", (success ? "success" : "FAIL"));

        // Test MergeSort::SortSimpleUnrolled
		memcpy(testData.get(), originalData.get(), sizeof(int) * dataLength);
        timer.Reset();
		mergeSorter.SortSimpleUnrolled(testData.get(), dataLength);
        ms = 1000.0f * timer.Time();
        printf("SortSimpleUnrolled time %f ms\n", ms);
		success = VerifyOrder(testData.get(), dataLength);
        printf("SortSimpleUnrolled %s\n", (success ? "success" : "FAIL"));

        // Test MergeSort::SortSimple
		memcpy(testData.get(), originalData.get(), sizeof(int) * dataLength);
        timer.Reset();
		mergeSorter.SortSimple(testData.get(), dataLength);
        ms = 1000.0f * timer.Time();
        printf("SortSimple time %f ms\n", ms);
		success = VerifyOrder(testData.get(), dataLength);
        printf("SortSimple %s\n", (success ? "success" : "FAIL"));
    }

    // STRINGS
    {
        const int dataLength = 2000001;
        std::unique_ptr<char[]> longString(new char[dataLength]);
        for(int i = 0; i < dataLength; ++i)
        {
            longString[i] = ' ' + (rand() & 0x3F); // only spaces, letters and some other characters, no zeros
        }
        longString[dataLength - 1] = 0; // null terminate

        timer.Reset();
		char commonLetter = GetMostCommonLetter(longString.get());
        ms = 1000.0f * timer.Time();
        printf("GetMostCommonLetter time %f ms\n", ms);
        printf("Letter is %c\n", commonLetter);

        timer.Reset();
        ReverseWords(longString.get());
        ms = 1000.0f * timer.Time();
        printf("ReverseWords time %f ms\n", ms);
    }


	// Cache
	{
		Cache<int, int> cache(5);

		for (int i = 0; i < 10; ++i)
		{
			cache.put(i, i);
		}
		int v = -1;
		bool success = true;
		success = !cache.get(0, v) && success;
		success = !cache.get(1, v) && success;
		success = !cache.get(2, v) && success;
		success = !cache.get(3, v) && success;
		success = !cache.get(4, v) && success;
		success = cache.get(9, v) && success;
		success = (v == 9) && success;
		success = cache.get(5, v) && success;
		success = (v == 5) && success;
		cache.put(1, 1);
		success = !cache.get(6, v) && success;
		success = cache.get(9, v) && success;
		success = (v == 9) && success;
		success = cache.get(5, v) && success;
		success = (v == 5) && success;
		printf("Cache %s\n", (success ? "success" : "FAIL"));
	}
    return 0;
}

