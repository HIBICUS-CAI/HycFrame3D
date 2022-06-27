#pragma once

#include <vector>

namespace Tool
{
    int Align(int _value, int _alignment);

    float Lerp(float _v1, float _v2, float _factor);

    float Clamp(float _v, float _min, float _max);

    float RandomVariance(float median, float variance);

    void CalcGaussWeight1D(uint16_t _kernelSize, float _sigma,
        std::vector<float>& _outResult);
}
