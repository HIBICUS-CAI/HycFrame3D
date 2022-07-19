#pragma once

#include "ActorAll.h"
#include "UiAll.h"

void registerMiscProcess(ObjectFactory *Factory);

bool dragonInit(AInteractComponent *);
void dragonUpdate(AInteractComponent *, const Timer &);
void dragonDestory(AInteractComponent *);

bool hillInfoInit(UInteractComponent *);
void hillInfoUpdate(UInteractComponent *, const Timer &);
void hillInfoDestory(UInteractComponent *);

bool resultInit(UInteractComponent *);
void resultUpdate(UInteractComponent *, const Timer &);
void resultDestory(UInteractComponent *);

bool logoFadeInit(UInteractComponent *);
void logoFadeUpdate(UInteractComponent *, const Timer &);
void logoFadeDestory(UInteractComponent *);
