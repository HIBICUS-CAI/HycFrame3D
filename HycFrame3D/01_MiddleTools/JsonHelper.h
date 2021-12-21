#pragma once

#include "rapidjson\document.h"
#include "rapidjson\istreamwrapper.h"
#include "rapidjson\pointer.h"
#include "rapidjson\filereadstream.h"
#include <fstream>
#include <string>

using JsonFile = rapidjson::Document;
using JsonNode = rapidjson::Value*;

void LoadJsonFile(JsonFile* json, std::string _path);

JsonNode GetJsonNode(JsonFile* _file, std::string _path);
