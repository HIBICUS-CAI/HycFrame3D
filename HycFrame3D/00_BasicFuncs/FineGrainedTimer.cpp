#include "FineGrainedTimer.h"

#include <cassert>

Timer::Timer()
    : Litmp({}), Qt1(0), Qt2(0), Dft(0.), Dff(0.), Dfm(0.), ActiveFlag(false) {
  (void)ActiveFlag;
}

Timer::~Timer() {}

void
Timer::timeIn() {
#ifdef _DEBUG
  assert(!ActiveFlag);
  ActiveFlag = true;
#endif // _DEBUG

  QueryPerformanceFrequency(&Litmp);
  Dff = static_cast<double>(Litmp.QuadPart);
  QueryPerformanceCounter(&Litmp);
  Qt1 = Litmp.QuadPart;
}

void
Timer::timeOut() {
  QueryPerformanceCounter(&Litmp);
  Qt2 = Litmp.QuadPart;
  Dfm = 1000. * static_cast<double>(Qt2 - Qt1);
  Dft = Dfm / Dff;

  if (Dft > 33.3333333333) {
    Dft = 33.3333333333;
  }

#ifdef _DEBUG
  assert(ActiveFlag);
  ActiveFlag = false;
#endif // _DEBUG
}

double
Timer::doubleDeltaTime() const {
  return Dft;
}

float
Timer::floatDeltaTime() const {
  return static_cast<float>(Dft);
}
