#pragma once

#include "ActorAll.h"
#include "UiAll.h"

void registerBulletProcess(ObjectFactory *Factory);

bool bulletManagerInit(AInteractComponent *);
void bulletManagerUpdate(AInteractComponent *, const Timer &);
void bulletManagerDestory(AInteractComponent *);

void setBulletShoot(dx::XMFLOAT3 &Pos, dx::XMFLOAT3 &Vec);

bool getPlayerCanAimFlg();

bool checkCollisionWithBullet(ACollisionComponent *Acc);
