#pragma once

#include <Windows.h>

class Timer {
private:
  LARGE_INTEGER Litmp;
  LONGLONG Qt1;
  LONGLONG Qt2;
  double Dft;
  double Dff;
  double Dfm;
  bool ActiveFlag;

public:
  Timer();
  ~Timer();

  void timeIn();

  void timeOut();

  double doubleDeltaTime() const;

  float floatDeltaTime() const;
};
