#include "stdafx.h"

#include "Timer.h"

#define WIN32_LEAN_AND_MEAN
#include "windows.h"


float Timer::s_period = 0.0f;


Timer::Timer()
{
    if(s_period == 0.0f)
    {
        uint64_t frequency;
        bool result = (QueryPerformanceFrequency((LARGE_INTEGER *) &frequency) > 0);
        if(result == true) 
        {
            s_period = 1.0f / (float)frequency;
        }
    }
    m_lastReset = m_lastTick = GetTimerTick();
}


uint64_t Timer::GetTimerTick()
{
  uint64_t tick;
  bool result = (QueryPerformanceCounter((LARGE_INTEGER *) &tick) > 0);
  if(result)
  {
      return tick;
  }
  return 0;
}


