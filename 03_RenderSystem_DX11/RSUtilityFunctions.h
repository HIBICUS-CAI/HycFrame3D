#pragma once

#include <vector>

namespace Tool
{
    int Align(int _value, int _alignment);

    float RandomVariance(float median, float variance);

    void CalcGaussWeight1D(uint16_t _kernelSize, float _sigma,
        std::vector<float>& _outResult);
}
