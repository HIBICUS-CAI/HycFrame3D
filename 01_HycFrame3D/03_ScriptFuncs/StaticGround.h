#pragma once

#include "ActorAll.h"

void RegisterStaticGround(ObjectFactory* _factory);

bool GoundInit(AInteractComponent*);
void GoundUpdate(AInteractComponent*, const Timer&);
void GoundDestory(AInteractComponent*);
