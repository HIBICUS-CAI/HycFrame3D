#pragma once

#include <vector>

namespace rs_tool {

int align(int Value, int Alignment);

float lerp(float V1, float V2, float Factor);

float clamp(float V, float Min, float Max);

float randomVariance(float Median, float Variance);

void calcGaussWeight1D(uint32_t KernelSize,
                       float Sigma,
                       std::vector<float> &OutResult);

} // namespace rs_tool
