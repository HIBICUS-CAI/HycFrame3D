#pragma once

#include "ActorAll.h"
#include "UiAll.h"

void RegisterMaterialEditor(ObjectFactory* _factory);

void MatEditorInput(AInputComponent*, Timer&);
bool MatEditorInit(AInteractComponent*);
void MatEditorUpdate(AInteractComponent*, Timer&);
void MatEditorDestory(AInteractComponent*);
