// Algorithms.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdlib.h>

#ifdef _DEBUG
#include <vld.h>
#endif

#include "ScopedArray.h"
#include "ComputePrimes.h"
#include "MergeSort.h"
#include "Timer.h"
#include "JobScheduler.h"
#include "GetMostCommonLetter.h"
#include "ReverseWords.h"


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
        int maxPrimes = 50000;
        ScopedArray<int> primes(new int[maxPrimes]);

        timer.Reset();
        ComputePrimesSimple(primes, maxPrimes);
        ms = 1000.0f * timer.Time();
        printf("Computed %d primes Simply in %f ms\n", maxPrimes, ms);

        timer.Reset();
        ComputePrimesFast(primes, maxPrimes);
        ms = 1000.0f * timer.Time();
        printf("Computed %d primes Fast in %f ms\n", maxPrimes, ms);
    }

    // SORTING
    {
        // Prepare data to sort. We use the same input data for each sort test.
        const int dataLength = 100000001;
        ScopedArray<int> originalData(new int[dataLength]);
        ScopedArray<int> testData(new int[dataLength]);
        for(int i = 0; i < dataLength; ++i)
        {
            originalData[i] = rand();
        }

        MergeSort<int> mergeSorter(dataLength);

        // Test MergeSort::Sort
        memcpy(testData, originalData, sizeof(int) * dataLength);
        timer.Reset();
        mergeSorter.Sort(testData, dataLength);
        ms = 1000.0f * timer.Time();
        printf("Sort time %f ms\n", ms);
        bool success = VerifyOrder(testData, dataLength);
        printf("Sort %s\n", (success ? "success" : "FAIL"));

        // test multithreaded sort
        {
            memcpy(testData, originalData, sizeof(int) * dataLength);

            // Create queue and threads for workers (we wil use our main thread too, so create one fewer worker).
            // This is scoped so that threads cease when test is over.
            const size_t NUM_THREADS_FOR_SORTING = 8;
            const int MAX_JOBS_IN_QUEUE = 16;
            JobScheduler jobScheduler(NUM_THREADS_FOR_SORTING - 1, MAX_JOBS_IN_QUEUE);

            timer.Reset();
            mergeSorter.SortSimpleMT(testData, dataLength, jobScheduler);
            ms = 1000.0f * timer.Time();
            printf("SortSimpleMT time %f ms\n", ms);
            bool success = VerifyOrder(testData, dataLength);
            printf("SortSimpleMT %s\n", (success ? "success" : "FAIL"));
        }

        // Test MergeSort::SortSimpleUnrolled
        memcpy(testData, originalData, sizeof(int) * dataLength);
        timer.Reset();
        mergeSorter.SortSimpleUnrolled(testData, dataLength);
        ms = 1000.0f * timer.Time();
        printf("SortSimpleUnrolled time %f ms\n", ms);
        success = VerifyOrder(testData, dataLength);
        printf("SortSimpleUnrolled %s\n", (success ? "success" : "FAIL"));

        // Test MergeSort::SortSimple
        memcpy(testData, originalData, sizeof(int) * dataLength);
        timer.Reset();
        mergeSorter.SortSimple(testData, dataLength);
        ms = 1000.0f * timer.Time();
        printf("SortSimple time %f ms\n", ms);
        success = VerifyOrder(testData, dataLength);
        printf("SortSimple %s\n", (success ? "success" : "FAIL"));
    }

    // STRINGS
    {
        const int dataLength = 100000001;
        ScopedArray<char> longString(new char[dataLength]);
        for(int i = 0; i < dataLength; ++i)
        {
            longString[i] = ' ' + (rand() & 0x3F); // only spaces, letters and some other characters, no zeros
        }
        longString[dataLength - 1] = 0; // null terminate

        timer.Reset();
        char commonLetter = GetMostCommonLetter(longString);
        ms = 1000.0f * timer.Time();
        printf("GetMostCommonLetter time %f ms\n", ms);
        printf("Letter is %c\n", commonLetter);

        timer.Reset();
        ReverseWords(longString);
        ms = 1000.0f * timer.Time();
        printf("ReverseWords time %f ms\n", ms);
    }

    return 0;
}

