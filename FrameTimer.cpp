#include "stdafx.h"
#include "FrameTimer.h"

using namespace std::chrono;

FrameTimer::FrameTimer()
{
    m_last = steady_clock::now();
}

float FrameTimer::Mark()
{
    auto old = m_last;

    m_last = steady_clock::now();

    const duration<float> frameTime = m_last - old;

    return frameTime.count();
}
