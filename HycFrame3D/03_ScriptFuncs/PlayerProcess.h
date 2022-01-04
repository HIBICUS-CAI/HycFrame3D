#pragma once

#include "ActorAll.h"

constexpr auto PLAYER_NAME = "player-actor";

void RegisterPlayerProcess(ObjectFactory* _factory);

void PlayerInput(AInputComponent*, Timer&);

bool PlayerInit(AInteractComponent*);
void PlayerUpdate(AInteractComponent*, Timer&);
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
