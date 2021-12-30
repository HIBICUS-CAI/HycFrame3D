#pragma once

#include "ActorAll.h"

void RegisterSPInput(ObjectFactory* _factory);

void TestASpInput(AInputComponent*, Timer&);

bool TestASpInit(AInteractComponent*);
void TestASpUpdate(AInteractComponent*, Timer&);
void TestASpDestory(AInteractComponent*);
