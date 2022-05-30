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

    namespace Json
    {
        void LoadJsonFile(JsonFile* json, const char* _path)
        {
            std::ifstream ifs(_path);
            rapidjson::IStreamWrapper isw(ifs);
            json->ParseStream(isw);
        }

        JsonNode GetJsonNode(JsonFile* _file, const char* _path)
        {
            char name[1024];
            sprintf_s(name, 1024, "%s", _path);

            rapidjson::Pointer ptr(name);

            return rapidjson::GetValueByPointer(*_file, ptr);
        }
    }
}
