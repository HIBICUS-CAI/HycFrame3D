#pragma once

#include "ActorAll.h"

void registerPlayer(ObjectFactory *Factory);

void playerInput(AInputComponent *, const Timer &);
bool playerInit(AInteractComponent *);
void playerUpdate(AInteractComponent *, const Timer &);
void playerDestory(AInteractComponent *);
