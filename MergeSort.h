#ifndef _MergeSort_h_
#define _MergeSort_h_

#include <assert.h>
#include <intrin.h>
#include <new>
#include <xutility>

#include "ScopedArray.h"
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
    volatile bool m_working;
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
    bool Sort(T* a_input, int a_length);

    // The same as Sort() but without a memcpy optimization
    bool SortSimpleUnrolled(T* a_input, int a_length);

    // The same as SortUnrolled() but doesn't unroll the first iteration of the main loop
    bool SortSimple(T* a_input, int a_length);

    // The same as SortSimple() but is multithreaded
    bool SortSimpleMT(T* a_input, int a_length, JobScheduler& a_jobScheduler);

private:

    // Merges two arrays from the source buffer (indexed by a_left and a_right) into the destination buffer.
    // a_right - a_left is the count of the left array and a_end - a_right is the count of the right array.
    // The sorted result in a_destinationBuffer starts at a_left and has a count of a_end - a_left.
    // We assume a precondition that the left buffer always has at least one item.
    // Optimizes by using memcpy to copy across the remaining source array when the other array is empty.
    inline void Merge(T* a_sourceBuffer, T* a_destinationBuffer, int a_left, int a_right, int a_end);

    // Returns false if a large enough buffer cannot be allocated.
    bool EnsureBufferIsLargeEnough(int a_length);

    T* m_scratchBuffer;
    int m_scratchLength; // number of T items that the buffer can contain
    bool m_autoAllocateScratch;
};


// The same as Merge() but more straightforward and not quite as fast 
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
bool MergeSort<T>::Sort(T* a_input, int a_length)
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
            Merge(buffer1, buffer2, i, std::min(i + width, a_length), std::min(i + (width << 1), a_length));
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

    for(int width = 1; width < context->m_dataLength; width <<= 1)
    {
        for(int i = 0; i < context->m_dataLength; i += (width << 1))
        {
            MergeSimple(context->m_buffer1, context->m_buffer2, i, std::min(i + width, context->m_dataLength), std::min(i + (width << 1), context->m_dataLength));
        }
        std::swap(context->m_buffer1, context->m_buffer2);
    }

    context->m_working = false;
}


template<class T>
void MergeJob(void* a_context)
{
    MergeContext<T>* context = (MergeContext<T>*) a_context;

    MergeSimple(context->m_buffer1, context->m_buffer2, 0, context->m_right, context->m_end);
    std::swap(context->m_buffer1, context->m_buffer2);

    context->m_working = false;
}


template<class T>
bool MergeSort<T>::SortSimpleMT(T* a_input, int a_length, JobScheduler& a_jobScheduler)
{

    // use a single thread for small sorts
    const int minItemsPerThread = 256;
    int totalNumThreads = a_jobScheduler.NumThreads() + 1; // including this thread
    if(a_length < totalNumThreads * minItemsPerThread)
    {
        return Sort(a_input, a_length);
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
    ScopedArray<SortContext<T> > sortContext(new SortContext<T>[totalNumThreads]);

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
    ScopedArray<MergeContext<T> > mergeContext(new MergeContext<T>[totalNumThreads]);
    for(int i = 0; i < totalNumThreads; ++i)
    {
        mergeContext[i].m_buffer1 = sortContext[i].m_buffer1;
        mergeContext[i].m_buffer2 = sortContext[i].m_buffer2;
        mergeContext[i].m_right = sortContext[i].m_dataLength;
        mergeContext[i].m_end = mergeContext[i].m_right;
    }

    int numMerges = a_jobScheduler.NumThreads();
    int totalMerges = totalNumThreads;
    while(totalMerges > 1)
    {
        numMerges = totalMerges >> 1;
        totalMerges = (totalMerges + 1) >> 1;

        for(int i = 0; i < totalMerges; ++i)
        {
            if(i < numMerges)
            {
                mergeContext[i].m_working = true;
                mergeContext[i].m_buffer1 = mergeContext[i << 1].m_buffer1;
                mergeContext[i].m_buffer2 = mergeContext[i << 1].m_buffer2;
                mergeContext[i].m_right = mergeContext[i << 1].m_end;
                mergeContext[i].m_end = mergeContext[i].m_right + mergeContext[(i << 1) + 1].m_end;

                Job job;
                job.m_data = &mergeContext[i];
                job.m_function = &MergeJob<T>;
                a_jobScheduler.SubmitJob(job);
            }
            else
            {
                // no work required
                mergeContext[i].m_buffer1 = mergeContext[i << 1].m_buffer1;
                mergeContext[i].m_buffer2 = mergeContext[i << 1].m_buffer2;
                mergeContext[i].m_right = mergeContext[i << 1].m_end;
                mergeContext[i].m_end = mergeContext[i].m_right;
            }
        }

        // wait for all merging to be done
        stillWorking = true;
        do
        {
            stillWorking = false;
            for(int i = 0; i < numMerges; ++i)
            {
                if(mergeContext[i].m_working)
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
inline void  MergeSort<T>::Merge(T* a_sourceBuffer, T* a_destinationBuffer, int a_left, int a_right, int a_end)
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
