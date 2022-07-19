#pragma once

#include "ActorAll.h"

void registerStaticGround(ObjectFactory *Factory);

bool groundInit(AInteractComponent *);
void groundUpdate(AInteractComponent *, const Timer &);
void groundDestory(AInteractComponent *);
