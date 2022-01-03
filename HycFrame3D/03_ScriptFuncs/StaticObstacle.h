#pragma once

#include "ActorAll.h"

void RegisterStaticObstacle(ObjectFactory* _factory);

bool ObstacleInit(AInteractComponent*);
void ObstacleUpdate(AInteractComponent*, Timer&);
void ObstacleDestory(AInteractComponent*);
