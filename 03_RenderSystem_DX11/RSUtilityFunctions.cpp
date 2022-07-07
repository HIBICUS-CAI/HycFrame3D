#include "RSUtilityFunctions.h"

#include <cassert>
#include <fstream>
#include <stdlib.h>
#include <time.h>

namespace rs_tool {

int align(int Value, int Alignment) {
  return (Value + (Alignment - 1)) & ~(Alignment - 1);
}

float lerp(float V1, float V2, float Factor) {
  return (1.f - Factor) * V1 + Factor * V2;
}

float clamp(float V, float Min, float Max) {
  V = (V > Max) ? Max : V;
  V = (V < Min) ? Min : V;
  return V;
}

float randomVariance(float Median, float Variance) {
  static bool HasInited = false;
  if (!HasInited) {
    srand(static_cast<uint32_t>(time(nullptr)));
    HasInited = true;
  }

  float FUnitRandomValue =
      static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  float FRange = Variance * FUnitRandomValue;
  return Median - Variance + (2.0f * FRange);
}

float gauss1D(float X, float Sigma) {
  return expf(-1.f * X * X / (2.f * Sigma * Sigma));
}

void calcGaussWeight1D(uint32_t KernelSize,
                       float Sigma,
                       std::vector<float> &OutResult) {
  assert(KernelSize % 2);
  OutResult.resize(KernelSize);
  int X = -1 * (KernelSize / 2);
  float Sum = 0.f;
  for (uint32_t I = 0; I < KernelSize; I++) {
    float weight = gauss1D(static_cast<float>(X++), Sigma);
    OutResult[I] = weight;
    Sum += weight;
  }

  for (auto &W : OutResult) {
    W /= Sum;
  }
}

} // namespace rs_tool
