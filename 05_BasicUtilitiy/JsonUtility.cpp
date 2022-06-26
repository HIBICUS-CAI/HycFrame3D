#include <rapidjson\istreamwrapper.h>
#include <rapidjson\filereadstream.h>
#include <fstream>

#include "JsonUtility.h"

using namespace hyc;

bool text::LoadAndParse(JsonFile& _outFile, const std::string& _path)
{
    std::ifstream ifs(_path);
    rapidjson::IStreamWrapper isw(ifs);
    _outFile.ParseStream(isw);

    return !_outFile.HasParseError();
}

bool text::LoadAndParse(JsonFile& _outFile, cstring _path)
{
    std::ifstream ifs(_path);
    rapidjson::IStreamWrapper isw(ifs);
    _outFile.ParseStream(isw);

    return !_outFile.HasParseError();
}

uint text::GetParseError(const JsonFile& _file)
{
    return static_cast<uint>(_file.GetParseError());
}

text::JsonNode text::GetNodeFromRoot(JsonFile& _file, const std::string& _path)
{
    rapidjson::Pointer ptr(_path.c_str());
    return rapidjson::GetValueByPointer(_file, ptr);
}

text::JsonNode text::GetNodeFromRoot(JsonFile& _file, cstring _path)
{
    rapidjson::Pointer ptr(_path);
    return rapidjson::GetValueByPointer(_file, ptr);
}
