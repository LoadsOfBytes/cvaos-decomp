/* Chaos orb: circles the core, opens when the core asks for an attack, breaks.
 * (failure block at the end via goto, nested
 * position tests so cross-jump shares the compare). */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/* allocation of 10 words in the Chaos core's unk_504 */
struct ChaosData {
    s32 graphics;
    s32 palette;
    s32 palette2;
    s32 bgRotation;
    s32 bgAngle;
    s32 bgScale;
    s32 bgRadius;
    struct EwramData_EntityData *orbs[3];
};

extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_GetVisibilityPattern(s32);
extern void Effect_CreateHomingOrb(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void ChaosOrb_CreateShellPiece(struct EwramData_EntityData *orb, s32 frame, s32 timer);
extern void ChaosOrb_SpawnSeedShot(struct EwramData_EntityData *orb, s32 x, s32 y, s32 delay);
extern void ChaosOrb_SpawnSpark(struct EwramData_EntityData *orb, s32 x, s32 y);
extern void ChaosOrb_SpawnSoulShot(struct EwramData_EntityData *orb, s32 x, s32 y);

#define OFS_X(e) (e->ext.chaosOrb.offsetX)
#define OFS_Y(e) (e->ext.chaosOrb.offsetY)
#define KIND(e) ((u8)e->ext.chaosOrb.kind)
#define REQUEST(c) (c->ext.chaos.activeOrbs)
#define BUSY(c) (c->ext.chaos.busy)

/**
 * @brief Orb update: ride on the core's circle with its halo and aura, attack
 * when the core allows it (seed shots, sparks or soul shots by kind) and
 * break apart when its HP is gone (state 10, shell pieces); delete with
 * the core.
 */
void ChaosOrb_Update(struct EwramData_EntityData *entity)
{
    u8 box[16];
    struct EwramData_EntityData *core = entity->ext.chaosChild.link;
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct ChaosData *data = (struct ChaosData *)core->ext.chaos.data.value;
    s32 x, y, limit;

    if (core == NULL || core->updateFunc == NULL || core->unk_518.part8.enemyId != entity->unk_518.part8.enemyId)
        goto fail;
    if (entity->state == 0)
    {
        x = (Sin(data->bgAngle + 0x4000) >> 14) + OFS_X(entity) + 2;
        y = (Sin(data->bgAngle) >> 14) + OFS_Y(entity) - 12;
        entity->posX.whole = Sin(0x4000 - (data->bgRotation + (u16)entity->ext.chaosOrb.angle)) * 60;
        entity->posY.whole = Sin(-(data->bgRotation + (u16)entity->ext.chaosOrb.angle)) * 60;
        entity->posX.part16.integer += x;
        entity->posY.part16.integer += y;
        x = entity->posX.whole + bg->xPos.whole;
        y = entity->posY.whole + bg->yPos.whole;
        switch (entity->subState)
        {
        case 0:
            if (KIND(entity) == 0)
            {
                if (!(REQUEST(core) & 1))
                    break;
                if (x > 0x7FFFFF)
                {
                    if (y <= 0xA00000)
                        goto busy;
                }
                else if (y <= 0x700000)
                    goto busy;
                entity->subState = 10;
                entity->stage = 0;
            }
            else if (KIND(entity) == 1)
            {
                if (!(REQUEST(core) & 2))
                    break;
                if (x > 0x7FFFFF)
                {
                    if ((u32)(y - 0x800001) > 0xFFFFE)
                        goto busy;
                }
                else if (y > 0x8FFFFF)
                    goto busy;
                entity->subState = 30;
                entity->stage = 0;
            }
            else if (KIND(entity) == 2)
            {
                if (!(REQUEST(core) & 4))
                    break;
                entity->subState = 20;
                entity->stage = 0;
            }
            break;
        case 10:
            switch (entity->stage)
            {
            case 0:
                entity->stage = 1;
                ChaosOrb_SpawnSeedShot(entity, 0, -0x300000, 20);
                ChaosOrb_SpawnSeedShot(entity, -0x200000, -0x140000, 50);
                ChaosOrb_SpawnSeedShot(entity, 0x200000, -0x140000, 80);
                entity->timer = 100;
                break;
            case 1:
                if ((s8)--entity->timer <= 0)
                {
                    entity->subState = 0;
                    BUSY(core) = 1;
                    REQUEST(core) &= ~1;
                }
                break;
            }
            break;
        case 20:
            switch (entity->stage)
            {
            case 0:
                entity->stage = 1;
                ChaosOrb_SpawnSpark(entity, 0, 0);
                entity->timer = 250;
                break;
            case 1:
                if (--entity->timer <= 1)
                {
                    entity->subState = 0;
                    BUSY(core) = 1;
                    REQUEST(core) &= ~4;
                }
                break;
            }
            break;
        case 30:
            switch (entity->stage)
            {
            case 0:
                entity->stage = 1;
                ChaosOrb_SpawnSoulShot(entity, 0, 0);
                entity->timer = 250;
                entity->hidden = 1;
                entity->hit.flags |= 4;
                entity->ext.chaosOrb.shotDone = 0;
                break;
            case 1:
                if (entity->ext.chaosOrb.shotDone)
                {
                    entity->subState = 0;
                    BUSY(core) = 1;
                    entity->hidden = 0;
                    entity->hit.flags &= ~4;
                    REQUEST(core) &= ~2;
                }
                break;
            }
            break;
        case 40:
            if (Entity_GetVisibilityPattern(entity->timer--) == 1)
                entity->palette = 4;
            else
                entity->palette = entity->unk_510.part8.basePalette;
            if (entity->timer == 0)
            {
                entity->subState = 0;
                entity->palette = entity->unk_510.part8.basePalette;
            busy:
                BUSY(core) = 1;
            }
            break;
        }
        Anim_Update(entity);
        Hit_CheckWithPlayer(box, entity);
        Enemy_UpdateHitFlash(entity);
    }
    else if (entity->state == 10)
    {
        x = (Sin(data->bgAngle + 0x4000) >> 14) + OFS_X(entity) + 2;
        y = (Sin(data->bgAngle) >> 14) + OFS_Y(entity) - 12;
        entity->posX.whole = Sin(0x4000 - (data->bgRotation + (u16)entity->ext.chaosOrb.angle)) * 60;
        entity->posY.whole = Sin(-(data->bgRotation + (u16)entity->ext.chaosOrb.angle)) * 60;
        entity->posX.part16.integer += x;
        entity->posY.part16.integer += y;
        if (entity->subState == 0)
        {
            entity->subState++;
            entity->hidden = 1;
            entity->timer = 120;
            ChaosOrb_CreateShellPiece(entity, 0, 20);
            ChaosOrb_CreateShellPiece(entity, 1, 32);
            ChaosOrb_CreateShellPiece(entity, 2, 44);
            PlaySong(0x71);
            switch (KIND(entity))
            {
            case 0:
                core->ext.chaos.brokenOrbs |= 1;
                break;
            case 1:
                core->ext.chaos.brokenOrbs |= 2;
                break;
            case 2:
                core->ext.chaos.brokenOrbs |= 4;
                break;
            }
        }
        else if (entity->subState == 1)
        {
            if (entity->timer > 0x50 && !(entity->timer & 7))
                Effect_CreateHomingOrb(gEwramData->playerState.player, (s16)entity->posX.part16.integer,
                             (s16)entity->posY.part16.integer, -0x100000, KIND(entity));
            if ((s8)--entity->timer <= 0)
                entity->subState++;
        }
        else if (entity->subState == 2)
        {
            switch (KIND(entity))
            {
            case 0:
                entity->deleteRequested = 1;
                gEwramData->playerState.actionLockFlags &= ~0x100;
                break;
            case 1:
                entity->deleteRequested = 1;
                gEwramData->playerState.actionLockFlags &= ~0x200;
                break;
            case 2:
                entity->deleteRequested = 1;
                gEwramData->playerState.actionLockFlags &= ~0x400;
                break;
            }
        }
    }
    return;
fail:
    entity->state = 10;
    entity->subState = 2;
}
