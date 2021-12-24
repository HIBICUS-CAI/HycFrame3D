#pragma once

#ifndef HYC_FRAME_3D
#define HYC_FRAME_3D
#endif // !HYC_FRAME_3D

#include "PrintLog.h"
#include <assert.h>
#include "FineGrainedTimer.h"

enum class OBJECT_TYPE
{
    ACTOR,
    UI,

    SIZE
};

enum class COMP_TYPE
{
    A_TRANSFORM,
    A_INPUT,
    A_INTERACT,
    A_TIMER,
    A_COLLISION,
    //A_SPRITEANI,
    A_MESH,
    A_LIGHT,
    //A_CAMERA,
    //A_PHYSICS,
    A_AUDIO,
    A_PARTICLE,
    U_TRANSFORM,
    U_SPRITE,
    U_ANIMATE,
    //U_TEXT,
    U_TIMER,
    U_INPUT,
    U_INTERACT,
    U_BUTTON,
    U_AUDIO,

    SIZE
};

enum class STATUS
{
    NEED_INIT,
    ACTIVE,
    PAUSE,
    NEED_DESTORY
};
