#pragma once

#include <rapidjson\document.h>
#include <rapidjson\pointer.h>
#include <string>

#include "HycType.h"

namespace hyc
{
    namespace text
    {
        using JsonFile = rapidjson::Document;
        using JsonNode = rapidjson::Value*;

        bool LoadAndParse(JsonFile& _outFile, const std::string& _path);
        bool LoadAndParse(JsonFile& _outFile, cstring _path);

        uint GetParseError(const JsonFile& _file);

        JsonNode GetNodeFromRoot(JsonFile& _file, const std::string& _path);
        JsonNode GetNodeFromRoot(JsonFile& _file, cstring _path);
    }
}
