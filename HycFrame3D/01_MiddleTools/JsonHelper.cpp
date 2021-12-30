#include "JsonHelper.h"
#include <vector>

const unsigned int MAX_NAME = 1024;

void LoadJsonFile(JsonFile* json, std::string _path)
{
    std::ifstream ifs(_path);
    rapidjson::IStreamWrapper isw(ifs);
    json->ParseStream(isw);
}

JsonNode GetJsonNode(JsonFile* _file, std::string _path)
{
    static char name[MAX_NAME];
    sprintf_s(name, MAX_NAME, "%s", _path.c_str());

    rapidjson::Pointer ptr(name);

    return rapidjson::GetValueByPointer(*_file, ptr);
}
