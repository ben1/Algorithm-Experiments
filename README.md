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
    | Multi-threaded (2 threads) |          4.902 |
    | Multi-threaded (4 threads) |          2.855 |
    | Multi-threaded (8 threads) |          1.995 |
    +----------------------------+----------------+

The multithreaded version splits the unsorted data into a number of lists equal to the number of threads. Each thread sorts its list independently. Then, pairs of neighboring lists are merged together so that the set of sorted lists has half the number of lists but each sorted list is twice the length. This continues until there is only one list. This list-merging is done in parallel too. But rather than use 4 threads to merge 8 lists into 4, we use 8! This is done by getting a thread to work to fill the destination list from the start and another to fill it from the end. This doubles the number of threads we can use for the list-merging part.

### Compute Primes
The most basic algorithm is O(n^2) so it is very slow (41 seconds to find 50000 primes).

The optimized algorithm is O(n log n) and takes 28 milliseconds to find 50000 primes.

### Most Common Letter In A String
Because the main loop doesn't have any branches and is pretty cache-friendly, this can't get much better without multithreading it. However even just checking the length of the string may take almost as long as the main loop, so being able to split up the string without this foreknowledge would not be worthwhile. It could be optimized with SSE instructions for streaming data from memory.

It takes 163 milliseconds to count the most common letter out of 100 million characters.

### Reverse Words
Because no memory is allocated, and the algorithm is O(n), it is pretty fast. It could be multi-threaded and use SSE streaming instructions.

It takes 227 milliseconds to reverse the words in a string of 100 million characters.

## TODO

* Make the multithreaded scheduler nicer to use.
* Make the threads wait on a signal when inactive
