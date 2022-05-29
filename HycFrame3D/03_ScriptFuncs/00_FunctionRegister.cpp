#include "00_FunctionRegister.h"
#include "01_MaterialEditor.h"
#include "SPInput.h"

void RegisterAllFuncPtr(ObjectFactory* _factory)
{
    RegisterMaterialEditor(_factory);
    RegisterSPInput(_factory);
}
