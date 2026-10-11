#include "FineGrainedTimer.h"

#include <cassert>

Timer::Timer() : Qt1(), Qt2(), Dft(0.), ActiveFlag(false) { (void)ActiveFlag; }

Timer::~Timer() {}

void Timer::timeIn() {
#ifdef _DEBUG
  assert(!ActiveFlag);
  ActiveFlag = true;
#endif // _DEBUG

  Qt1 = Clock::now();
}

void Timer::timeOut() {
  Qt2 = Clock::now();
  Dft = std::chrono::duration<double, std::milli>(Qt2 - Qt1).count();

  if (Dft > 33.3333333333) {
    Dft = 33.3333333333;
  }

#ifdef _DEBUG
  assert(ActiveFlag);
  ActiveFlag = false;
#endif // _DEBUG
}

double Timer::doubleDeltaTime() const { return Dft; }

float Timer::floatDeltaTime() const { return static_cast<float>(Dft); }
