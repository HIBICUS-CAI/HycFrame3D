#pragma once

#include "ActorAll.h"

void registerDestProcess(ObjectFactory *Factory);

bool destInit(AInteractComponent *);
void destUpdate(AInteractComponent *, const Timer &);
void destDestory(AInteractComponent *);

bool destPtcInit(AInteractComponent *);
void destPtcUpdate(AInteractComponent *, const Timer &);
void destPtcDestory(AInteractComponent *);
