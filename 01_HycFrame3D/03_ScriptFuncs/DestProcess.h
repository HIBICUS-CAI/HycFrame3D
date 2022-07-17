#pragma once

#include "ActorAll.h"

void RegisterDestProcess(ObjectFactory* _factory);

bool DestInit(AInteractComponent*);
void DestUpdate(AInteractComponent*, const Timer&);
void DestDestory(AInteractComponent*);

bool DestPtcInit(AInteractComponent*);
void DestPtcUpdate(AInteractComponent*, const Timer&);
void DestPtcDestory(AInteractComponent*);
