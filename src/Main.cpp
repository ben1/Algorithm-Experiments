// Algorithms.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <memory>
#include <stdlib.h>
#include <stack>

#ifdef _DEBUG
#include <vld.h>
#endif

#include "BinarySearchTree.h"
#include "ComputePrimes.h"
#include "MergeSort.h"
#include "Timer.h"
#include "JobQueue.h"
#include "GetMostCommonLetter.h"
#include "ReverseWords.h"
#include "Cache.h"
#include "ConvertBase.h"


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

	// BINARY SEARCH TREE
	{
		typedef BinarySearchTree<int> BSTInt;
		BSTInt tree;
		tree.Insert(15);
		tree.Insert(3);
		tree.Insert(19);
		tree.Insert(4);
		tree.Insert(7);
		tree.Insert(16);
		tree.Insert(2);
		BSTInt::Node* n = tree.GetRoot();
		std::stack<BSTInt::Node*> stack;
		while (n != 0)
		{
			// if we can go left, push the current node and go
			if (n->m_left != 0)
			{
				stack.push(n);
				n = n->m_left;
			}
			else
			{
				// can't go left, so print
				printf(" %d", n->m_value);
				while (!stack.empty() && n->m_right == 0)
				{
					n = stack.top();
					stack.pop();

					// the new value we popped is the next in order
					printf(" %d", n->m_value);
				}
				n = n->m_right;
			}
		}

		// link up each node's m_next pointer with the node to the right of it at the same depth in the tree.
		for (int row = 0;; ++row)
		{
			// for each row of depth
			int depth = 0;
			BSTInt::Node* n = tree.GetRoot();

			// structure for remembering unsearched left node paths
			struct LastBranch { int m_depth; BSTInt::Node* m_node; };
			std::stack<LastBranch> lastLeft;

			// remember the right-most newly visited node at the desired depth
			BSTInt::Node* lastRight = 0;

			// now find the next node at the same level starting from the right
			while (lastRight == 0 || !lastLeft.empty())
			{
				// if there is an unsearched left branch, try that
				if (!lastLeft.empty())
				{
					n = lastLeft.top().m_node;
					depth = lastLeft.top().m_depth;
					lastLeft.pop();
				}

				// iterate down the tree to find the right-most node at the appropriate row.
				while (depth < row)
				{
					// prefer right paths if possible
					if (n->m_right != 0)
					{
						// save the last left branch we could have taken
						if (n->m_left != 0)
						{
							lastLeft.push(LastBranch());
							lastLeft.top().m_depth = depth + 1;
							lastLeft.top().m_node = n->m_left;
						}

						n = n->m_right;
					}
					else if (n->m_left != 0)
					{
						n = n->m_left;
					}
					else
					{
						break;
					}
					++depth;
				}
				if (depth == row)
				{
					n->m_next = lastRight;
					lastRight = n;
				}
				else if (lastLeft.empty() && lastRight == 0)
				{
					// there is no node at this row, so we've finished
					break;
				}
			}
			
			// there was no node at this row, so we've finished
			if (lastRight == 0 && lastLeft.empty())
			{
				break;
			}
		}
		n = tree.GetRoot();
	}

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
			JobQueue jobScheduler(NUM_THREADS_FOR_SORTING - 1);

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

		char sss[] = { "one two three four five" };
		ReverseWords(sss);
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

	{
		std::string hex;
		bool success = ConvertBase("255", hex);
		success = (hex == "FF") && success;
		success = ConvertBase("586697456878621459", hex) && success;
		success = (hex == "8245E3A83E01313") && success;
		printf("ConvertBase %s\n", (success ? "success" : "FAIL"));
	}

    return 0;
}

