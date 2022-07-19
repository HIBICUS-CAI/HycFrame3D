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

void registerPlayerProcess(ObjectFactory *Factory);

void playerInput(AInputComponent *, const Timer &);

bool playerInit(AInteractComponent *);
void playerUpdate(AInteractComponent *, const Timer &);
void playerDestory(AInteractComponent *);

void setPlayerDashFlg(bool CanDashFlag);
bool getPlayerDashFlg();

bool getPlayerIsDashingFlg();

bool getPlayerAimingFlg();

void setPlayerContactGround();

void setPlayerBrokeHead();

void setPlayerDashToObstacle();

const DirectX::XMFLOAT3 &getPlayerMoveDirection();
void setPlayerMoveDirection(const DirectX::XMFLOAT3 &Direction);

void setPlayerLastReachGround(ATransformComponent *GroundAtc);

void resetDeadPlayerToGround();
