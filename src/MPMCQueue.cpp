/*  Multi-producer/multi-consumer bounded queue.
 *  Copyright (c) 2010-2011, Dmitry Vyukov. All rights reserved.
 *  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *     1. Redistributions of source code must retain the above copyright notice, this list of
 *        conditions and the following disclaimer.
 *     2. Redistributions in binary form must reproduce the above copyright notice, this list
 *        of conditions and the following disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *  THIS SOFTWARE IS PROVIDED BY DMITRY VYUKOV "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 *  DMITRY VYUKOV OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *  The views and conclusions contained in the software and documentation are those of the authors and should not be interpreted
 *  as representing official policies, either expressed or implied, of Dmitry Vyukov.
 */ 

#include "stdafx.h"

#include "MPMCQueue.h"



size_t const thread_count = 8;
size_t const batch_size = 1;
size_t const iter_count = 2000000;

bool volatile g_start = 0;

typedef mpmc_bounded_queue<int> queue_t;


unsigned __stdcall thread_func(void* ctx)
{
    queue_t& queue = *(queue_t*)ctx;
    int data;

    srand((unsigned)time(0) + GetCurrentThreadId());
    size_t pause = rand() % 1000;

    while (g_start == 0)
        SwitchToThread();

    for (size_t i = 0; i != pause; i += 1)
        _mm_pause();

    for (int iter = 0; iter != iter_count; ++iter)
    {
        for (size_t i = 0; i != batch_size; i += 1)
        {
            while (!queue.enqueue(i))
                SwitchToThread();
        }
        for (size_t i = 0; i != batch_size; i += 1)
        {
            while (!queue.dequeue(data))
            SwitchToThread();
        }
    }

    return 0;
}


bool mpmc_bounded_queue_test()
{
    queue_t queue (1024);

    HANDLE threads [thread_count];
    for (int i = 0; i != thread_count; ++i)
    {
        threads[i] = (HANDLE)_beginthreadex(0, 0, thread_func, &queue, 0, 0);
    }

    Sleep(1);

    unsigned __int64 start = __rdtsc();
    g_start = 1;

    WaitForMultipleObjects(thread_count, threads, 1, INFINITE);

    unsigned __int64 end = __rdtsc();
    unsigned __int64 time = end - start;
    std::cout << "cycles/op=" << time / (batch_size * iter_count * 2 * thread_count) << std::endl;

    return true;
}
