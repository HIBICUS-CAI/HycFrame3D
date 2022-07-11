#pragma once

#include "ActorAll.h"
#include "UiAll.h"

void registerMaterialEditor(ObjectFactory *Factory);

void matEditorInput(AInputComponent *, const Timer &);
bool matEditorInit(AInteractComponent *);
void matEditorUpdate(AInteractComponent *, const Timer &);
void matEditorDestory(AInteractComponent *);
