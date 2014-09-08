#ifndef _Timer_h_
#define _Timer_h_


class Timer
{
public:
  Timer();

  // Reset the time to 0 seconds
  inline void Reset();

  // Return the time in seconds since the last call to Tick
  inline float Tick();

  /// \brief return the time in seconds since the last Reset
  inline float Time();

private:
  uint64_t GetTimerTick();

  static float s_period;
  uint64_t m_lastTick;
  uint64_t m_lastReset;
};


inline void Timer::Reset()
{
  m_lastReset = m_lastTick = GetTimerTick();
}


inline float Timer::Tick()
{
    uint64_t current = GetTimerTick();
    float time = s_period * (float) (current - m_lastTick);
    m_lastTick = current;
    return time;
}


inline float Timer::Time()
{
    return s_period * (float) (GetTimerTick() - m_lastReset);
}


#endif
