#pragma once

#include <chrono>

class Timer {
private:
  using Clock = std::chrono::steady_clock;

  Clock::time_point Qt1;
  Clock::time_point Qt2;
  double Dft;
  bool ActiveFlag;

public:
  Timer();
  ~Timer();

  void timeIn();

  void timeOut();

  double doubleDeltaTime() const;

  float floatDeltaTime() const;
};
