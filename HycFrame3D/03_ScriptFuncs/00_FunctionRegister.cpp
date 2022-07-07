#include "00_FunctionRegister.h"

#include "01_MaterialEditor.h"
#include "SPInput.h"

void
registerAllFuncPtr(ObjectFactory *Factory) {
  registerMaterialEditor(Factory);
  registerSPInput(Factory);
}
