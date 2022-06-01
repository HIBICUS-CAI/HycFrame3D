#include "00_FunctionRegister.h"
#include "SPInput.h"
#include "GM02.h"

void RegisterAllFuncPtr(ObjectFactory* _factory)
{
    RegisterSPInput(_factory);
    RegisterGM02(_factory);
}
