/* Chronomage waiting-state transitions. */
#include "code_08009A0.h"
#include "player_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Chronomage_UpdateWait(struct EwramData_EntityData *);
extern void Chronomage_UpdateCast(struct EwramData_EntityData *);
extern void Chronomage_UpdateEscape(struct EwramData_EntityData *);
extern void Chronomage_UpdateBestiaryPreview(struct EwramData_EntityData *);
extern void Chronomage_OnHit(struct EwramData_EntityData *);
extern u8 sChronomageGfx[], sChronomagePalette[], sChronomageSpriteSheet[];

#include "code/code_08060B98.h"
extern s32 Target_GetX(struct EwramData_EntityData *);
extern u8 sChronomageWalkProfile[];
#define ABS_CHRONOMAGE(v) ((v) < 0 ? -(v) : (v))

/**
 * @brief Wait state (0): enter escape while the global stop timer is active;
 * otherwise begin casting on player proximity or player flags 0x360.
 */
void Chronomage_UpdateWait(struct EwramData_EntityData *entity)
{
    s32 playerX = (s16)gEwramData->playerState.player->posX.part16.integer;
    s16 distance;
    if (playerX - (s16)entity->posX.part16.integer < 0) distance = (u16)entity->posX.part16.integer - playerX;
    else distance = playerX - (u16)entity->posX.part16.integer;
    if (gEwramData->gameState.timeStopTimer != 0)
    {
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
    else if (distance <= 0x6F || (PLAYER_ACTION_FLAGS(gEwramData->playerState.player) & 0x360))
    {
        entity->state = 1;
        entity->stage = 0;
        entity->subState = 0;
    }
}

