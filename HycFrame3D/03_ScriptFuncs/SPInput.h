#pragma once

#include "ActorAll.h"
#include "UiAll.h"

void registerSPInput(ObjectFactory *Factory);

void testASpInput(AInputComponent *, const Timer &);

bool testASpInit(AInteractComponent *);
void testASpUpdate(AInteractComponent *, const Timer &);
void testASpDestory(AInteractComponent *);

void testUSpInput(UInputComponent *, const Timer &);
void testUSpBtnInput(UInputComponent *, const Timer &);

bool testUSpInit(UInteractComponent *);
void testUSpUpdate(UInteractComponent *, const Timer &);
void testUSpDestory(UInteractComponent *);

void tempToTitle(AInputComponent *, const Timer &);
void tempToSelect(AInputComponent *, const Timer &);
void tempToRun(AInputComponent *, const Timer &);
void tempToResult(AInputComponent *, const Timer &);

bool aniInit(AInteractComponent *);
void aniUpdate(AInteractComponent *, const Timer &);
void aniDestory(AInteractComponent *);

bool bbInit(AInteractComponent *);
void bbUpdate(AInteractComponent *, const Timer &);
void bbDestory(AInteractComponent *);
