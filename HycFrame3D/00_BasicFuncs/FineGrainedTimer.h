#pragma once

#include <Windows.h>

class Timer
{
public:
    Timer(void);
    ~Timer(void);

    void TimeIn();
    void TimeOut();

    double DoubleDeltaTime() const;
    float FloatDeltaTime() const;

private:
    LARGE_INTEGER mLitmp;
    LONGLONG mQt1;
    LONGLONG mQt2;
    double mDft;
    double mDff;
    double mDfm;
    bool mActiveFlg;
};
