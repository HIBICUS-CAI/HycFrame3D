#pragma once

#include "ActorAll.h"

void registerStaticObstacle(ObjectFactory *Factory);

bool obstacleInit(AInteractComponent *);
void obstacleUpdate(AInteractComponent *, const Timer &);
void obstacleDestory(AInteractComponent *);
