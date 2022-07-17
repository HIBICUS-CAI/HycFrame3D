#pragma once

#include "ActorAll.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#endif // __clang__

constexpr auto PLAYER_NAME = "player-actor";

#if __clang__
#pragma clang diagnostic pop
#endif // __clang__

void RegisterPlayerProcess(ObjectFactory* _factory);

void PlayerInput(AInputComponent*, const Timer&);

bool PlayerInit(AInteractComponent*);
void PlayerUpdate(AInteractComponent*, const Timer&);
void PlayerDestory(AInteractComponent*);

void SetPlayerDashFlg(bool _canDashFlg);
bool GetPlayerDashFlg();

bool GetPlayerIsDashingFlg();

bool GetPlayerAimingFlg();

void SetPlayerContactGround();

void SetPlayerBrokeHead();

void SetPlayerDashToObstacle();

DirectX::XMFLOAT3& GetPlayerMoveDirection();
void SetPlayerMoveDirection(DirectX::XMFLOAT3 _dir);

void SetPlayerLastReachGround(ATransformComponent* _groundAtc);

void ResetDeadPlayerToGround();
