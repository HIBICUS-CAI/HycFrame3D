#pragma once

#include "ActorAll.h"

void RegisterTitleCamera(ObjectFactory* _factory);

bool TitleCamInit(AInteractComponent*);
void TitleCamUpdate(AInteractComponent*, const Timer&);
void TitleCamDestory(AInteractComponent*);
