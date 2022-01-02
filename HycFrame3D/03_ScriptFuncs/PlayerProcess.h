#pragma once

#include "ActorAll.h"

constexpr auto PLAYER_NAME = "player-actor";

void RegisterPlayerProcess(ObjectFactory* _factory);

void PlayerMove(AInputComponent*, Timer&);

bool PlayerInit(AInteractComponent*);
void PlayerUpdate(AInteractComponent*, Timer&);
void PlayerDestory(AInteractComponent*);

void SetPlayerDashFlg(bool _canDashFlg);
bool GetPlayerDashFlg();

bool GetPlayerAimingFlg();

void SetPlayerContactGround();
