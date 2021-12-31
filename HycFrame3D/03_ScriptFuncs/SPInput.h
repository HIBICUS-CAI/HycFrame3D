#pragma once

#include "ActorAll.h"
#include "UiAll.h"

void RegisterSPInput(ObjectFactory* _factory);

void TestASpInput(AInputComponent*, Timer&);

bool TestASpInit(AInteractComponent*);
void TestASpUpdate(AInteractComponent*, Timer&);
void TestASpDestory(AInteractComponent*);

void TestUSpInput(UInputComponent*, Timer&);
void TestUSpBtnInput(UInputComponent*, Timer&);

bool TestUSpInit(UInteractComponent*);
void TestUSpUpdate(UInteractComponent*, Timer&);
void TestUSpDestory(UInteractComponent*);
