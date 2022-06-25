#pragma once

#include "rapidjson\document.h"
#include "rapidjson\pointer.h"

#include <vector>

namespace Tool
{
    int Align(int _value, int _alignment);

    float RandomVariance(float median, float variance);

    void CalcGaussWeight1D(uint16_t _kernelSize, float _sigma,
        std::vector<float>& _outResult);

    namespace Json
    {
        using JsonFile = rapidjson::Document;
        using JsonNode = rapidjson::Value*;

        void LoadJsonFile(JsonFile* json, const char* _path);

        JsonNode GetJsonNode(JsonFile* _file, const char* _path);
    }
}
