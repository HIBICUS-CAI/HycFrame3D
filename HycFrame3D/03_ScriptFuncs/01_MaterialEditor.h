#pragma once

#include "ActorAll.h"
#include "UiAll.h"

void
registerMaterialEditor(ObjectFactory *Factory);

void
matEditorInput(AInputComponent *, Timer &);
bool
matEditorInit(AInteractComponent *);
void
matEditorUpdate(AInteractComponent *, Timer &);
void
matEditorDestory(AInteractComponent *);
