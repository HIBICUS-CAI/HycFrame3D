#include "FineGrainedTimer.h"
#include <assert.h>

Timer::Timer(void) :
    mLitmp({}), mQt1(0), mQt2(0), mDft(0.), mDff(0.), mDfm(0.), mActiveFlg(false)
{}


Timer::~Timer(void) {}

void Timer::TimeIn()
{
#ifdef _DEBUG
    assert(!mActiveFlg);
    mActiveFlg = true;
#endif // _DEBUG

    QueryPerformanceFrequency(&mLitmp);
    mDff = (double)mLitmp.QuadPart;
    QueryPerformanceCounter(&mLitmp);
    mQt1 = mLitmp.QuadPart;
}

void Timer::TimeOut()
{
    QueryPerformanceCounter(&mLitmp);
    mQt2 = mLitmp.QuadPart;
    mDfm = 1000. * (double)(mQt2 - mQt1);
    mDft = mDfm / mDff;

    if (mDft > 16.6666666667f)
    {
        mDft = 16.6666666667f;
    }

#ifdef _DEBUG
    assert(mActiveFlg);
    mActiveFlg = false;
#endif // _DEBUG
}

double Timer::DoubleDeltaTime() const
{
    return mDft;
}

float Timer::FloatDeltaTime() const
{
    return (float)mDft;
}
