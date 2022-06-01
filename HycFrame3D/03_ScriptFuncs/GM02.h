#pragma once

#include "ActorAll.h"
#include "UiAll.h"

void RegisterGM02(ObjectFactory* _factory);

void PlayerInput(AInputComponent*, Timer&);
bool PlayerInit(AInteractComponent*);
void PlayerUpdate(AInteractComponent*, Timer&);
void PlayerDestory(AInteractComponent*);

bool BulletInit(AInteractComponent*);
void BulletUpdate(AInteractComponent*, Timer&);
void BulletDestory(AInteractComponent*);
