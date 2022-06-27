#include "RSUtilityFunctions.h"
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include "rapidjson\istreamwrapper.h"
#include "rapidjson\filereadstream.h"

namespace Tool
{
    int Align(int _value, int _alignment)
    {
        return (_value + (_alignment - 1)) & ~(_alignment - 1);
    }

    float Lerp(float _v1, float _v2, float _factor)
    {
        return (1.f - _factor)* _v1 + _factor * _v2;
    }

    float Clamp(float _v, float _min, float _max)
    {
        _v = (_v > _max) ? _max : _v;
        _v = (_v < _min) ? _min : _v;
        return _v;
    }

    float RandomVariance(float median, float variance)
    {
        static bool hasInited = false;
        if (!hasInited)
        {
            srand((unsigned int)time(nullptr));
            hasInited = true;
        }

        float fUnitRandomValue = (float)rand() / (float)RAND_MAX;
        float fRange = variance * fUnitRandomValue;
        return median - variance + (2.0f * fRange);
    }

    float Gauss1D(float _x, float _sigma)
    {
        return expf(-1.f * _x * _x / (2.f * _sigma * _sigma));
    }

    void CalcGaussWeight1D(uint16_t _kernelSize, float _sigma,
        std::vector<float>& _outResult)
    {
        assert(_kernelSize % 2);
        _outResult.resize(_kernelSize);
        int x = -1 * (_kernelSize / 2);
        float sum = 0.f;
        for (uint16_t i = 0; i < _kernelSize; i++)
        {
            float weight = Gauss1D((float)(x++), _sigma);
            _outResult[i] = weight;
            sum += weight;
        }

        for (auto& w : _outResult) { w /= sum; }
    }
}
