#ifndef PLAYER_EFFECTS_H
#define PLAYER_EFFECTS_H

#include "types.h"
#include "structs/ewram.h"
#include "structs/main.h"

void PlayerEffectSetPart_Update(struct EwramData_EntityData *entity);
void Player_SpawnEffectSet(s32 set, s32 facing);
void Effect_UpdatePlayerSurfaceEffect(struct EwramData_EntityData *entity);
void Effect_UpdateFallingDebris(struct EwramData_EntityData *entity);
void Player_SpawnSurfaceEffect(s32 kind);
void Effect_SpawnDebrisBurst(struct EwramData_EntityData *entity);
void PushBlockGroup_Create(struct EwramData_EntityData *entity);
void PushBlock_Update(struct EwramData_EntityData *block);
s16 PushBlockGroup_CeilingCorrection(struct EwramData_EntityData *entity, s16 x, s16 y);
s16 PushBlockGroup_FloorCorrection(struct EwramData_EntityData *entity, s16 x, s16 y);
s16 PushBlockGroup_WallRightCorrection(struct EwramData_EntityData *entity, s16 x, s16 y);
s16 PushBlockGroup_WallLeftCorrection(struct EwramData_EntityData *entity, s16 x, s16 y);
void PushBlockGroup_DeleteIfOffscreen(struct EwramData_EntityData *entity);
void PushBlockGroup_Delete(struct EwramData_EntityData *entity);
void PushBlockGroupCreate(struct EwramData_EntityData *entity);
void PushBlockGroupUpdate(struct EwramData_EntityData *entity);

#endif /* PLAYER_EFFECTS_H */
