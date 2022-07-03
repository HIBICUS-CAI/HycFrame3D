#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpragma-once-outside-header"
#pragma once
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#include <rapidjson\document.h>
#include <rapidjson\pointer.h>
#include <rapidjson\istreamwrapper.h>
#include <rapidjson\filereadstream.h>
#pragma clang diagnostic pop
#include <string>
#include <fstream>

#include "HycType.h"

namespace Hyc
{
    namespace Text
    {
        using JsonFile = rapidjson::Document;
        using JsonNode = rapidjson::Value*;

        inline bool LoadJsonAndParse(JsonFile& _outFile, const std::string& _path)
        {
            std::ifstream ifs(_path);
            rapidjson::IStreamWrapper isw(ifs);
            _outFile.ParseStream(isw);

            return !_outFile.HasParseError();
        }

        inline bool LoadJsonAndParse(JsonFile& _outFile, cstring _path)
        {
            std::ifstream ifs(_path);
            rapidjson::IStreamWrapper isw(ifs);
            _outFile.ParseStream(isw);

            return !_outFile.HasParseError();
        }

        inline uint GetJsonParseError(const JsonFile& _file)
        {
            return static_cast<uint>(_file.GetParseError());
        }

        inline JsonNode GetJsonNode(JsonFile& _file, const std::string& _path)
        {
            rapidjson::Pointer ptr(_path.c_str());
            return rapidjson::GetValueByPointer(_file, ptr);
        }

        inline JsonNode GetJsonNode(JsonFile& _file, cstring _path)
        {
            rapidjson::Pointer ptr(_path);
            return rapidjson::GetValueByPointer(_file, ptr);
        }

        template <typename T>
        T GetAs(const JsonNode& _node)
        {
            return _node->Get<T>();
        }
    }
}
