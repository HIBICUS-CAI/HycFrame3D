#pragma once

#include <string>
#include "RSCommon.h"

enum class MODEL_FILE_TYPE
{
    BIN,
    JSON
};

struct MODEL_TEXTURE_INFO
{
    std::string mType = "";
    std::string mPath = "";
};

void LoadModelFile(const std::string _filePath, MODEL_FILE_TYPE _type,
    RS_SUBMESH_DATA* _result);

void AddBumpedTexTo(RS_SUBMESH_DATA* _result, std::string _filePath);
