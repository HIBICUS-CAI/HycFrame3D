#pragma once

#include "ObjectFactory.h"
#include "SPInput.h"

void RegisterAllFuncPtr(ObjectFactory* _factory)
{
    RegisterSPInput(_factory);
}
