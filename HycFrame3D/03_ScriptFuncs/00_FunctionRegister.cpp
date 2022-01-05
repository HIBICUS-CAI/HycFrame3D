#include "00_FunctionRegister.h"
#include "SPInput.h"
#include "PlayerProcess.h"
#include "NormalCrystal.h"
#include "SleepCrystal.h"
#include "BulletProcess.h"
#include "StaticGround.h"
#include "StaticObstacle.h"
#include "FadeProcess.h"
#include "PauseMenu.h"
#include "DestProcess.h"
#include "ButtonProcess.h"
#include "TitleCamera.h"

void RegisterAllFuncPtr(ObjectFactory* _factory)
{
    RegisterSPInput(_factory);
    RegisterPlayerProcess(_factory);
    RegisterNormalCrystal(_factory);
    RegisterSleepCrystal(_factory);
    RegisterBulletProcess(_factory);
    RegisterStaticGround(_factory);
    RegisterStaticObstacle(_factory);
    RegisterFadeProcess(_factory);
    RegisterPauseMenu(_factory);
    RegisterDestProcess(_factory);
    RegisterButtonProcess(_factory);
    RegisterTitleCamera(_factory);
}
