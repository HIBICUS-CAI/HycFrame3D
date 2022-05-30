#pragma once

#include "rapidjson\document.h"
#include "rapidjson\pointer.h"

namespace Tool
{
    int Align(int _value, int _alignment);

    float RandomVariance(float median, float variance);

    namespace Json
    {
        using JsonFile = rapidjson::Document;
        using JsonNode = rapidjson::Value*;

        void LoadJsonFile(JsonFile* json, const char* _path);

        JsonNode GetJsonNode(JsonFile* _file, const char* _path);
    }
}
