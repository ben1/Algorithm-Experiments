#ifndef _MergeSort_h_
#define _MergeSort_h_

#include <algorithm>
#include <assert.h>
#include <intrin.h>
#include <new>
#include <memory>
#include <xutility>

#include "JobScheduler.h"


template<class T>
struct SortContext
{
    T* m_buffer1; // source left array
    T* m_buffer2; // dest left array
    int m_dataLength; // total length of the array to be sorted
    volatile bool m_working;
};


template<class T>
struct MergeContext
{
    T* m_buffer1; // source left array
    T* m_buffer2; // dest left array
    int m_right; // offset to the right array (left array has offset 0 and ends just before the right array)
    int m_end; // offset to the end of the right array
    volatile bool m_workingFirst;
    volatile bool m_workingSecond;
};


template<class T>
class MergeSort
{
public:
    // Automatically allocate scratch buffer as needed
    MergeSort();

    // Pre-allocate scratch buffer to the given number of T items, and then auto allocate more if needed.
    MergeSort(int a_scratchLength);

    // Use the pre-allocated scratch buffer which can contain the given numbe of T items.
    // Sorting may fail if the given buffer is too small.
    MergeSort(T* a_scratchBuffer, int a_scratchLength);

    ~MergeSort();

    // Sorts the input buffer, which contains the given number of T items.
    // Returns false if the scratch buffer was not big enough.
    bool SortUnrolledMemcpy(T* a_input, int a_length);

    // The same as SortUnrolledMemcpy() but without a memcpy optimization
    bool SortSimpleUnrolled(T* a_input, int a_length);

    // The same as SortUnrolled() but doesn't unroll the first iteration of the main loop
    bool SortSimple(T* a_input, int a_length);

    // The same as SortSimpleUnrolled() but is multithreaded.
    bool SortMT(T* a_input, int a_length, JobScheduler& a_jobScheduler);

private:

    // Returns false if a large enough buffer cannot be allocated.
    bool EnsureBufferIsLargeEnough(int a_length);

    T* m_scratchBuffer;
    int m_scratchLength; // number of T items that the buffer can contain
    bool m_autoAllocateScratch;
};


// Merges two arrays from the source buffer (indexed by a_left and a_right) into the destination buffer.
// a_right - a_left is the count of the left array and a_end - a_right is the count of the right array.
// The sorted result in a_destinationBuffer starts at a_left and has a count of a_end - a_left.
// We assume a precondition that the left buffer always has at least one item.
// Optimizes by using memcpy to copy across the remaining source array when the other array is empty.
template<class T>
inline void MergeMemcpy(T* a_sourceBuffer, T* a_destinationBuffer, int a_left, int a_right, int a_end)
{
    a_destinationBuffer += a_left;
    T* left = a_sourceBuffer + a_left;
    T* right = a_sourceBuffer + a_right;
    T* endLeft = right;
    T* endRight = a_sourceBuffer + a_end;

    // We assume a precondition that the left buffer always has at least one item
    assert(left < endLeft);

    // There may be nothing in the right input buffer so check this case first.
    if(right >= endRight)
    {
        memcpy(a_destinationBuffer, left, (char*)endLeft - (char*)left);
        return;
    }

    while(true)
    {
        // compare left and right input buffers and copy the smaller value to the destination buffer.
        if(*left < *right)
        {
            *a_destinationBuffer = *left;
            a_destinationBuffer++;
            left++;
            // if we have exhausted the left buffer, copy the remainder of the right and return
            if(left >= endLeft)
            {
                memcpy(a_destinationBuffer, right, (char*)endRight - (char*)right);
                return;
            }
        }
        else
        {
            *a_destinationBuffer = *right;
            a_destinationBuffer++;
            right++;
            // if we have exhausted the right buffer, copy the remainder of the left and return
            if(right >= endRight)
            {
                memcpy(a_destinationBuffer, left, (char*)endLeft - (char*)left);
                return;
            }
        }
    }
}


// Functions the same as MergeMemcpy() but more straightforward and not quite as fast.
template<class T>
inline void MergeSimple(T* a_sourceBuffer, T* a_destinationBuffer, int a_left, int a_right, int a_end)
{
    T* left = a_sourceBuffer + a_left;
    T* right = a_sourceBuffer + a_right;
    T* endLeft = right;
    T* endRight = a_sourceBuffer + a_end;
    T* destinationBufferEnd = a_destinationBuffer + a_end;
    a_destinationBuffer += a_left;

    // We assume a precondition that the left buffer always has at least one item
    assert(left < endLeft);

    do
    {
        if(left < endLeft && (right >= endRight || *left < *right))
        {
            *a_destinationBuffer = *left;
            left++;
        }
        else
        {
            *a_destinationBuffer = *right;
            right++;
        }
        a_destinationBuffer++;
    }
    while(a_destinationBuffer < destinationBufferEnd);
}


// The same as MergeSimple() but only merges into the first half of the destination buffer.
// Used for the multi-threaded merge so that 2 threads can work on a single merge.
template<class T>
inline void MergeFirst(T* a_sourceBuffer, T* a_destinationBuffer, int a_left, int a_right, int a_end)
{
    T* left = a_sourceBuffer + a_left;
    T* right = a_sourceBuffer + a_right;
    T* endLeft = right;
    T* endRight = a_sourceBuffer + a_end;
    T* destinationBufferEnd = a_destinationBuffer + a_left + ((a_end - a_left + 1) >> 1); // only fill the first half, rounded up
    a_destinationBuffer += a_left;

    // We assume a precondition that the left buffer always has at least one item
    assert(left < endLeft);

    do
    {
        if(left < endLeft && (right >= endRight || *left < *right))
        {
            *a_destinationBuffer = *left;
            left++;
        }
        else
        {
            *a_destinationBuffer = *right;
            right++;
        }
        a_destinationBuffer++;
    }
    while(a_destinationBuffer < destinationBufferEnd);
}


// The same as MergeFirst() but merges from the other end of the lists.
// Used for the multi-threaded merge so that 2 threads can work on a single merge.
template<class T>
inline void MergeSecond(T* a_sourceBuffer, T* a_destinationBuffer, int a_left, int a_right, int a_end)
{
    // all pointers are set up to start from the end of the buffers and iterate towards the start.
    T* left = a_sourceBuffer + a_right - 1;
    T* right = a_sourceBuffer + a_end - 1;
    T* endLeft = a_sourceBuffer - 1;
    T* endRight = left;
    // the destination end is one less that the end position of MergeFirst()
    T* destinationBufferEnd = a_destinationBuffer + ((a_end - a_left + 1) >> 1) - 1;
    a_destinationBuffer += a_end - 1; // start from last entry

    // condition must go at the front for the case where the list only had one entry.
    while(a_destinationBuffer > destinationBufferEnd)
    {
        if(left > endLeft && (right <= endRight || *left > *right))
        {
            *a_destinationBuffer = *left;
            left--;
        }
        else
        {
            *a_destinationBuffer = *right;
            right--;
        }
        a_destinationBuffer--;
    }
}


template<class T>
MergeSort<T>::MergeSort()
    : m_scratchBuffer(0)
    , m_scratchLength(0)
    , m_autoAllocateScratch(true)
{
}


template<class T>
MergeSort<T>::MergeSort(int a_scratchLength)
    : m_scratchLength(a_scratchLength)
    , m_autoAllocateScratch(true)
{
    m_scratchBuffer = new (std::nothrow) T[a_scratchLength];
    if(m_scratchBuffer == 0)
    {
        m_scratchLength = 0;
    }
}


template<class T>
MergeSort<T>::MergeSort(T* a_scratchBuffer, int a_scratchLength)
    : m_scratchBuffer(a_scratchBuffer)
    , m_scratchLength(a_scratchLength)
    , m_autoAllocateScratch(false)
{
}


template<class T>
MergeSort<T>::~MergeSort()
{
    if(m_autoAllocateScratch && m_scratchBuffer != 0)
    {
        delete [] m_scratchBuffer;
    }
}


template<class T>
bool MergeSort<T>::SortUnrolledMemcpy(T* a_input, int a_length)
{
    // skip the trivial case
    if(a_length < 2)
    {
        return true;
    }

    // prepare scratch buffer
    if(!EnsureBufferIsLargeEnough(a_length))
    {
        return false;
    }

    // Keep a copy of the input array pointer and the scratch array. 
    // These pointers are swapped for each iteration to avoid copying.
    T* buffer1 = a_input;
    T* buffer2 = m_scratchBuffer;

    // unroll the first iteration because we can make some shortcuts
    for(int i = 0; i < a_length; i += 2)
    {
        if(i + 1 < a_length)
        {
            // compare left and right
            if(buffer1[i] < buffer1[i + 1])
            {
                // order is correct, so copy both elements
                memcpy(buffer2 + i, buffer1 + i, sizeof(T) << 1);
            }
            else
            {
                // swap as we copy
                buffer2[i] = buffer1[i + 1];
                buffer2[i + 1] = buffer1[i];
            }
        }
        else
        {
            // just copy the left
            buffer2[i] = buffer1[i];
        }
    }
    std::swap(buffer1, buffer2);

    for(int width = 2; width < a_length; width <<= 1)
    {
        for(int i = 0; i < a_length; i += (width << 1))
        {
            MergeMemcpy(buffer1, buffer2, i, std::min(i + width, a_length), std::min(i + (width << 1), a_length));
        }
        std::swap(buffer1, buffer2);
    }

    // if the results are in the scratch buffer, copy them back into the input buffer
    if(buffer1 == m_scratchBuffer)
    {
        memcpy(a_input, buffer1, sizeof(T) * a_length);
    }

    return true;
}

template<class T>
bool MergeSort<T>::SortSimpleUnrolled(T* a_input, int a_length)
{
    if(!EnsureBufferIsLargeEnough(a_length))
    {
        return false;
    }

    // Keep a copy of the input array pointer and the scratch array. 
    // These pointers are swapped for each iteration to avoid copying.
    T* buffer1 = a_input;
    T* buffer2 = m_scratchBuffer;

    // unroll the first iteration because we can make some shortcuts
    for(int i = 0; i < a_length; i += 2)
    {
        if(i + 1 < a_length)
        {
            // compare left and right
            if(buffer1[i] < buffer1[i + 1])
            {
                // order is correct, so copy both elements
                memcpy(buffer2 + i, buffer1 + i, sizeof(T) << 1);
            }
            else
            {
                // swap as we copy
                buffer2[i] = buffer1[i + 1];
                buffer2[i + 1] = buffer1[i];
            }
        }
        else
        {
            // just copy the left
            buffer2[i] = buffer1[i];
        }
    }
    std::swap(buffer1, buffer2);

    for(int width = 2; width < a_length; width <<= 1)
    {
        for(int i = 0; i < a_length; i += (width << 1))
        {
            MergeSimple(buffer1, buffer2, i, std::min(i + width, a_length), std::min(i + (width << 1), a_length));
        }
        std::swap(buffer1, buffer2);
    }

    // if the results are in the scratch buffer, copy them back into the input buffer
    if(buffer1 == m_scratchBuffer)
    {
        memcpy(a_input, buffer1, sizeof(T) * a_length);
    }

    return true;
}


template<class T>
bool MergeSort<T>::SortSimple(T* a_input, int a_length)
{
    if(!EnsureBufferIsLargeEnough(a_length))
    {
        return false;
    }

    // Keep a copy of the input array pointer and the scratch array. 
    // These pointers are swapped for each iteration to avoid copying.
    T* buffer1 = a_input;
    T* buffer2 = m_scratchBuffer;

    for(int width = 1; width < a_length; width <<= 1)
    {
        for(int i = 0; i < a_length; i += (width << 1))
        {
            MergeSimple(buffer1, buffer2, i, std::min(i + width, a_length), std::min(i + (width << 1), a_length));
        }
        std::swap(buffer1, buffer2);
    }

    // if the results are in the scratch buffer, copy them back into the input buffer
    if(buffer1 == m_scratchBuffer)
    {
        memcpy(a_input, buffer1, sizeof(T) * a_length);
    }

    return true;
}


template<class T>
void SortJob(void* a_context)
{
    SortContext<T>* context = (SortContext<T>*) a_context;

    // unroll the first iteration because we can make some shortcuts
    for(int i = 0; i < context->m_dataLength; i += 2)
    {
        if(i + 1 < context->m_dataLength)
        {
            // compare left and right
            if(context->m_buffer1[i] < context->m_buffer1[i + 1])
            {
                // order is correct, so copy both elements
                memcpy(context->m_buffer2 + i, context->m_buffer1 + i, sizeof(T) << 1);
            }
            else
            {
                // swap as we copy
                context->m_buffer2[i] = context->m_buffer1[i + 1];
                context->m_buffer2[i + 1] = context->m_buffer1[i];
            }
        }
        else
        {
            // just copy the left
            context->m_buffer2[i] = context->m_buffer1[i];
        }
    }
    std::swap(context->m_buffer1, context->m_buffer2);

    for(int width = 2; width < context->m_dataLength; width <<= 1)
    {
        for(int i = 0; i < context->m_dataLength; i += (width << 1))
        {
            MergeMemcpy(context->m_buffer1, context->m_buffer2, i, std::min(i + width, context->m_dataLength), std::min(i + (width << 1), context->m_dataLength));
        }
        std::swap(context->m_buffer1, context->m_buffer2);
    }

    context->m_working = false;
}


template<class T>
void MergeFirstJob(void* a_context)
{
    MergeContext<T>* context = (MergeContext<T>*) a_context;

    MergeFirst(context->m_buffer1, context->m_buffer2, 0, context->m_right, context->m_end);
    std::swap(context->m_buffer1, context->m_buffer2);

    context->m_workingFirst = false;
}


template<class T>
void MergeSecondJob(void* a_context)
{
    MergeContext<T>* context = (MergeContext<T>*) a_context;

    MergeSecond(context->m_buffer1, context->m_buffer2, 0, context->m_right, context->m_end);
    // don't need to swap the buffers because we ignore the buffers coming from the second jobs

    context->m_workingSecond = false;
}


template<class T>
bool MergeSort<T>::SortMT(T* a_input, int a_length, JobScheduler& a_jobScheduler)
{

    // use a single thread for small sorts
    const int minItemsPerThread = 256;
    int totalNumThreads = a_jobScheduler.NumThreads() + 1; // including this thread
    if(a_length < totalNumThreads * minItemsPerThread)
    {
        return SortUnrolledMemcpy(a_input, a_length);
    }

    if(!EnsureBufferIsLargeEnough(a_length))
    {
        return false;
    }

    // Keep a copy of the input array pointer and the scratch array. 
    // These pointers are swapped for each iteration to avoid copying.
    T* buffer1 = a_input;
    T* buffer2 = m_scratchBuffer;

    // First divide the list into a number of lists equal to the number of threads and sort them.
    int maxItemsPerThread = (a_length + totalNumThreads - 1) / totalNumThreads;
    std::unique_ptr<SortContext<T>[]> sortContext(new SortContext<T>[totalNumThreads]);

    // Dispatch work to threads
    int itemsDispatched = 0;
    for (int i = 0; i < totalNumThreads; ++i)
    {
        int itemsForThread = std::min(maxItemsPerThread, a_length - itemsDispatched);
        sortContext[i].m_working = true;
        sortContext[i].m_buffer1 = buffer1 + itemsDispatched;
        sortContext[i].m_buffer2 = buffer2 + itemsDispatched;
        sortContext[i].m_dataLength = itemsForThread;
        itemsDispatched += itemsForThread;

        if(i < a_jobScheduler.NumThreads())
        {
            Job job;
            job.m_data = &sortContext[i];
            job.m_function = &SortJob<T>;
            a_jobScheduler.SubmitJob(job);
        }
        else
        {
            SortJob<T>(&sortContext[i]);
        }
    }
    
    // wait for all sorting to be done
    bool stillWorking = true;
    do
    {
        stillWorking = false;
        for(int i = 0; i < a_jobScheduler.NumThreads(); ++i)
        {
            if(sortContext[i].m_working)
            {
                stillWorking = true;
                JobScheduler::YieldCurrentThread();
                break;
            }
        }
    }
    while(stillWorking);

    // copy sorted contexts into merge contexts
	std::unique_ptr<MergeContext<T>[]> mergeContext(new MergeContext<T>[totalNumThreads]);
    for(int i = 0; i < totalNumThreads; ++i)
    {
        mergeContext[i].m_buffer1 = sortContext[i].m_buffer1;
        mergeContext[i].m_buffer2 = sortContext[i].m_buffer2;
        mergeContext[i].m_right = sortContext[i].m_dataLength;
        mergeContext[i].m_end = sortContext[i].m_dataLength;
    }

    int totalMerges = totalNumThreads;
    while(totalMerges > 1)
    {
        // there must be an even number of merges. Any left-over list can't be merged this iteration.
        int numMerges = totalMerges >> 1;
        totalMerges = (totalMerges + 1) >> 1;

        for(int i = 0; i < totalMerges; ++i)
        {
            if(i < numMerges)
            {
                // take the results of the previous iteration and create new merge contexts by merging 2 together.
                mergeContext[i].m_workingFirst = true;
                mergeContext[i].m_workingSecond = true;
                mergeContext[i].m_buffer1 = mergeContext[i << 1].m_buffer1;
                mergeContext[i].m_buffer2 = mergeContext[i << 1].m_buffer2;
                mergeContext[i].m_right = mergeContext[i << 1].m_end;
                mergeContext[i].m_end = mergeContext[i].m_right + mergeContext[(i << 1) + 1].m_end;

                // submit 2 jobs for the merge context, each working from the other end of the list
                {
                    Job job;
                    job.m_data = &mergeContext[i];
                    job.m_function = &MergeFirstJob<T>;
                    a_jobScheduler.SubmitJob(job);
                }
                if((i << 1) + 1 < a_jobScheduler.NumThreads())
                {
                    Job job;
                    job.m_data = &mergeContext[i];
                    job.m_function = &MergeSecondJob<T>;
                    a_jobScheduler.SubmitJob(job);
                }
                else
                {
                    MergeSecondJob<T>(&mergeContext[i]);
                }
            }
            else
            {
                // no work required
                mergeContext[i].m_buffer1 = mergeContext[i << 1].m_buffer1;
                mergeContext[i].m_buffer2 = mergeContext[i << 1].m_buffer2;
                mergeContext[i].m_right = mergeContext[i << 1].m_end;
                mergeContext[i].m_end = mergeContext[i << 1].m_end;
            }
        }

        // wait for all merging to be done
        stillWorking = true;
        do
        {
            stillWorking = false;
            for(int i = 0; i < numMerges; ++i)
            {
                if(mergeContext[i].m_workingFirst || mergeContext[i].m_workingSecond)
                {
                    stillWorking = true;
                    JobScheduler::YieldCurrentThread();
                    break;
                }
            }
        }
        while(stillWorking);
    }

    // if the results are in the scratch buffer, copy them back into the input buffer
    if(mergeContext[0].m_buffer1 == m_scratchBuffer)
    {
        memcpy(a_input, mergeContext[0].m_buffer1, sizeof(T) * a_length);
    }

    return true;
}


template<class T>
bool MergeSort<T>::EnsureBufferIsLargeEnough(int a_length)
{
    if(m_scratchLength < a_length)
    {
        if(!m_autoAllocateScratch)
        {
            return false;
        }
        if(m_scratchBuffer != 0)
        {
            delete [] m_scratchBuffer;
        }
        m_scratchBuffer = new (std::nothrow) T[a_length];
        if(m_scratchBuffer != 0)
        {
            m_scratchLength = a_length;
        }
        else
        {
            m_scratchLength = 0;
            return false;
        }
    }
    return true;
}


#endif
