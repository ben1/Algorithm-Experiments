# Algorithm Experiments
This project is just a little C++ experiment with the goal of trying out some algorithm implementations and testing them for speed and correctness.

## Framework
The framework is extremely simple, consisting of a few utility classes:

* Scoped pointers
* Performance Timer
* Job Scheduler
* Leak detection using [VLD](http://vld.codeplex.com/)

## Algorithms

### Merge Sort
A straightforward implementation was done first, and then several optimizations attempted.
With identical random data input of 100 million integers, the results are: 

    +----------------------------+----------------+
    |              Algorithm     | Time (seconds) |
    +----------------------------+----------------+
    | Simple                     |         11.178 |
    | Unrolled                   |         10.599 |
    | UnrolledMemcpy             |          9.052 |
    | Multi-threaded (8 threads) |          2.377 |
    +----------------------------+----------------+

### Compute Primes
The most basic algorithm is O(n^2) so it is very slow (41 seconds to find 50000 primes).

The optimized algorithm is O(n log n) and takes 28 milliseconds to find 50000 primes.

### Get Most Common Letter
Because the main loop doesn't have any branches and is pretty cache-friendly, this can't get much better without multithreading it. However even just checking the length of the string may take almost as long as the main loop, so being able to split up the string without this foreknowledge would not be worthwhile.

It takes 163 milliseconds to count the most common letter out of 100 million characters.

### Reverse Words
Because no memory is allocated, and the algorithm is O(n), it is pretty fast.

It takes 227 milliseconds to reverse the words in a string of 100 million characters.

## TODO

* Add more comments
* Make the multithreaded scheduler nicer to use!
* Make the threads wait on a signal when inactive
* Keep track of what jobs have completed which parts of the sort by using atomic max / min

Like this:

    inline void AtomicMax(volatile long& a_value, long a_value2)
    {
        while(true)
        {
            long oldValue = a_value;
            long newValue = std::max(a_value2, oldValue);
            if(_InterlockedCompareExchange(&a_value, newValue, oldValue) == oldValue)
            {
                break;
            }
        }
    }


    inline void AtomicMin (volatile long& a_value, long a_value2)
    {
        while(true)
        {
            long oldValue = a_value;
            long newValue = std::min(a_value2, oldValue);
            if(_InterlockedCompareExchange(&a_value, newValue, oldValue) == oldValue)
            {
                break;
            }
        }
    }

