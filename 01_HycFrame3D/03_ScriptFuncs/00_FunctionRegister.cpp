#include "00_FunctionRegister.h"

#include "01_MaterialEditor.h"
#include "SPInput.h"

#include "BulletProcess.h"
#include "ButtonProcess.h"
#include "DestProcess.h"
#include "FadeProcess.h"
#include "MiscProcess.h"
#include "NormalCrystal.h"
#include "PauseMenu.h"
#include "PlayerProcess.h"
#include "SleepCrystal.h"
#include "StaticGround.h"
#include "StaticObstacle.h"
#include "TitleCamera.h"

void registerAllFuncPtr(ObjectFactory *Factory) {
  registerMaterialEditor(Factory);
  registerSPInput(Factory);
  registerPlayerProcess(Factory);
  registerNormalCrystal(Factory);
  registerSleepCrystal(Factory);
  registerBulletProcess(Factory);
  registerStaticGround(Factory);
  registerStaticObstacle(Factory);
  registerFadeProcess(Factory);
  registerPauseMenu(Factory);
  registerDestProcess(Factory);
  registerButtonProcess(Factory);
  registerTitleCamera(Factory);
  registerMiscProcess(Factory);
}
