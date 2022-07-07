#pragma once

#include "ActorAll.h"
#include "UiAll.h"

void registerSPInput(ObjectFactory *Factory);

void testASpInput(AInputComponent *, Timer &);

bool testASpInit(AInteractComponent *);
void testASpUpdate(AInteractComponent *, Timer &);
void testASpDestory(AInteractComponent *);

void testUSpInput(UInputComponent *, Timer &);
void testUSpBtnInput(UInputComponent *, Timer &);

bool testUSpInit(UInteractComponent *);
void testUSpUpdate(UInteractComponent *, Timer &);
void testUSpDestory(UInteractComponent *);

void tempToTitle(AInputComponent *, Timer &);
void tempToSelect(AInputComponent *, Timer &);
void tempToRun(AInputComponent *, Timer &);
void tempToResult(AInputComponent *, Timer &);

bool aniInit(AInteractComponent *);
void aniUpdate(AInteractComponent *, Timer &);
void aniDestory(AInteractComponent *);

bool bbInit(AInteractComponent *);
void bbUpdate(AInteractComponent *, Timer &);
void bbDestory(AInteractComponent *);
