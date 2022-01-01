#pragma once

#include "ActorAll.h"

void RegisterPlayerProcess(ObjectFactory* _factory);

void PlayerMove(AInputComponent*, Timer&);

bool PlayerInit(AInteractComponent*);
void PlayerUpdate(AInteractComponent*, Timer&);
void PlayerDestory(AInteractComponent*);
