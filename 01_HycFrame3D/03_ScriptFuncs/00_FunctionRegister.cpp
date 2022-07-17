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
  RegisterPlayerProcess(Factory);
  RegisterNormalCrystal(Factory);
  RegisterSleepCrystal(Factory);
  RegisterBulletProcess(Factory);
  RegisterStaticGround(Factory);
  RegisterStaticObstacle(Factory);
  RegisterFadeProcess(Factory);
  RegisterPauseMenu(Factory);
  RegisterDestProcess(Factory);
  RegisterButtonProcess(Factory);
  RegisterTitleCamera(Factory);
  RegisterMiscProcess(Factory);
}
