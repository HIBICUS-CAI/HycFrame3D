#pragma once

#include "ActorAll.h"

void RegisterDestProcess(ObjectFactory* _factory);

bool DestInit(AInteractComponent*);
void DestUpdate(AInteractComponent*, Timer&);
void DestDestory(AInteractComponent*);

bool DestPtcInit(AInteractComponent*);
void DestPtcUpdate(AInteractComponent*, Timer&);
void DestPtcDestory(AInteractComponent*);
