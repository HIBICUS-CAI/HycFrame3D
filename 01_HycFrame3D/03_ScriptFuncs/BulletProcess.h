#pragma once

#include "ActorAll.h"
#include "UiAll.h"

void RegisterBulletProcess(ObjectFactory* _factory);

bool BulletManagerInit(AInteractComponent*);
void BulletManagerUpdate(AInteractComponent *, const Timer &);
void BulletManagerDestory(AInteractComponent*);

void SetBulletShoot(DirectX::XMFLOAT3& _pos, DirectX::XMFLOAT3& _vec);

bool GetPlayerCanAimFlg();

bool CheckCollisionWithBullet(ACollisionComponent* _acc);
