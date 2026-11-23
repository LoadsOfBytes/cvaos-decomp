/* Chronomage escape, bestiary preview, creation and state dispatch. */
#include "code_08009A0.h"
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
 * @brief Escape state (2): disable hit flag 4, animate and accelerate away
 * from the target, jumping at terrain obstacles. Request deletion offscreen.
 */
void Chronomage_UpdateEscape(struct EwramData_EntityData *entity)
{
    s32 speed;
    s32 flags;
    switch (entity->subState)
    {
    case 0:
        entity->hit.flags &= ~4;
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->subState = 1;
        /* fall through */
    case 1:
        if (entity->animFlags & 4) entity->subState = 2;
        break;
    case 2:
        entity->flipX = 0;
        entity->accelY.whole = 0x2000;
        if (Target_GetX(entity) > entity->posX.whole) Anim_StartIndexed(entity, 4, 0, 1);
        else Anim_StartIndexed(entity, 5, 0, 1);
        entity->subState = 3;
        /* fall through */
    case 3:
        if ((s32)entity->ext.chronomage.escapeSpeed <= 0x2FFFF) entity->ext.chronomage.escapeSpeed += 0x2000;
        if (entity->animScriptIndex == 4) { speed = entity->ext.chronomage.escapeSpeed; speed = -speed; }
        else speed = entity->ext.chronomage.escapeSpeed;
        flags = Entity_Walk(entity, speed, sChronomageWalkProfile, 2);
        if ((flags & 8) && (flags & 3)) entity->velY.whole -= 0x30000;
        break;
    }
    if (EntityIsOutsideScreen(entity, 3)) entity->deleteRequested = 1;
}

/**
 * @brief Bestiary state (4): wait 128 updates, then sequence animations 1
 * and 2. Updates the animation internally.
 */
void Chronomage_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->subState = 1;
        entity->timer = 0x80;
        break;
    case 1:
        if (entity->timer != 0) entity->timer--;
        else
        {
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->subState = 2;
        }
        break;
    case 2:
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->subState = 3;
        }
        break;
    }
    Anim_Update(entity);
}

void EnemyChronomageCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sChronomageGfx, sChronomagePalette, 0, sChronomageSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x13;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview) entity->state = 4;
    else
    {
        Hit_Enable(entity, 8, Enemy_OnContact, (s32)Chronomage_OnHit);
        HitBox_SetBodyRaw(entity, 0x2008E0FC);
        HitBox_SetAttackRaw(entity, 0x2008E0FC);
        entity->hit.flags |= 4;
        Entity_SnapToGround(entity);
        entity->posY.part16.integer++;
        entity->scrollWithBg = 1;
        entity->state = 0;
    }
}

void EnemyChronomageUpdate(struct EwramData_EntityData *entity)
{
    u32 box[4];
    s32 done;
    if (entity->state != 3) Enemy_UpdateHitFlash(entity);
    switch (entity->state)
    {
    case 0: Chronomage_UpdateWait(entity); break;
    case 1: Chronomage_UpdateCast(entity); break;
    case 2: Chronomage_UpdateEscape(entity); break;
    case 3:
        switch (entity->subState)
        {
        case 0:
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->subState = 1;
            entity->timer = 0x28;
            goto alive;
        case 1:
            Enemy_UpdateFragmentDeath(entity);
            if (entity->animFlags & 4)
            {
                done = 1;
                break;
            }
        default:
alive:
            done = 0;
            break;
        }
        if (done)
        {
            EntityDelete(entity);
            return;
        }
        break;
    case 4:
        Chronomage_UpdateBestiaryPreview(entity);
        return;
    }
    if (entity->state != 3) Hit_CheckWithPlayer(box, entity);
    Anim_Update(entity);
}
