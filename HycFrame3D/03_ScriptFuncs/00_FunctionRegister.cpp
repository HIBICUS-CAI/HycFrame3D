#include "00_FunctionRegister.h"
#include "SPInput.h"
#include "PlayerProcess.h"

void RegisterAllFuncPtr(ObjectFactory* _factory)
{
    RegisterSPInput(_factory);
    RegisterPlayerProcess(_factory);
}
