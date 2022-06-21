#include "JsonHelper.h"
#include <vector>

void LoadJsonFile(JsonFile* json, std::string _path)
{
    std::ifstream ifs(_path);
    rapidjson::IStreamWrapper isw(ifs);
    json->ParseStream(isw);
}

JsonNode GetJsonNode(JsonFile* _file, std::string _path)
{
    rapidjson::Pointer ptr(_path.c_str());
    return rapidjson::GetValueByPointer(*_file, ptr);
}
