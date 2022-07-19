#pragma once

#include "ActorAll.h"

void registerTitleCamera(ObjectFactory *Factory);

bool titleCamInit(AInteractComponent *);
void titleCamUpdate(AInteractComponent *, const Timer &);
void titleCamDestory(AInteractComponent *);
