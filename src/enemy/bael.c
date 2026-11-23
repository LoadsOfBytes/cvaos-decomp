/*
 * Bael: charge, death, charge effect and movement (the active state is in the previous, separately compiled file; the head and Bael_SpawnChargeEffect are in the next one: the charge code passes s32 coordinates to its u16 parameters).
 */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include <string.h>
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"

/* Bael accelerated charge, rotation, and sparks. */
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *entity);
extern s32 Entity_Walk(struct EwramData_EntityData *entity,s32 speed,void *profile,s32 flags);
extern void Bael_RestoreXAtProbe(struct EwramData_EntityData *entity,s32 work);
extern void Bael_SpawnChargeEffect(s32 x,s32 y,s32 index,s32 facing);
extern s32 Bael_OffsetBySign(s32 value,s32 step);
extern s16 sBaelWalkProfile[];
#define SPEED(e) (e->ext.wheel.speed)
#define ANGLE(e) (e->ext.wheel.angle)
#define DIRECTION(e) (e->ext.wheel.direction)

/**
 * @brief Accelerate and rotate toward the sampled side, emit effects and count completed charges in the linked head.
 */
void Bael_UpdateCharge(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 work = entity->stage;
    s32 reset;
    switch (work)
    {
        case 0:
            if ((s16)Entity_GetTargetDistanceX(entity) > 0)
                DIRECTION(entity) = work;
            else
                DIRECTION(entity) = 1;
            entity->timer = 80;
            entity->stage++;
        case 1:
        {
            u32 timer;
            reset = entity->posX.whole;
            /* Keep the pre-movement coordinate in r4 across collision handling.
             * Reusing reset below also preserves the later zero's lifetime. */
            {
                register s32 oldX asm("r4") = reset; /* FAKEMATCH: pinned register */
                Entity_Walk(entity, SPEED(entity), sBaelWalkProfile, 6);
                Bael_RestoreXAtProbe(entity, oldX);
            }
            if (DIRECTION(entity))
            {
                SPEED(entity) += 0x1998;
                if ((s32)SPEED(entity) > 0x20000)
                {
                    SPEED(entity) = 0x20000;
                    entity->timer = 0;
                }
                if ((s32)SPEED(entity) > 0)
                    part->flipX = 1;
                if (entity->timer > 50)
                    ANGLE(entity) += 0x400;
                else
                    ANGLE(entity) += 0x800;
            }
            else
            {
                SPEED(entity) -= 0x1998;
                if ((s32)SPEED(entity) < -0x20000)
                {
                    SPEED(entity) = -0x20000;
                    entity->timer = 0;
                }
                if ((s32)SPEED(entity) < 0)
                    part->flipX = 0;
                if (entity->timer > 50)
                    ANGLE(entity) -= 0x400;
                else
                    ANGLE(entity) -= 0x800;
            }
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,0,0,entity->affineIndex);
            timer = entity->timer;
            if ((u8)(timer % 3) == 0 && timer > 50)
            {
                s16 *x;
                u16 *y;
                s32 initialX;
                if ((u8)(timer % 9) == 0)
                    PlaySong(0x84);
                x = (s16 *)&entity->posX.part16.integer;
                initialX = *x;
                y = &entity->posY.part16.integer;
                Bael_SpawnChargeEffect(initialX,(s16)(*y+24),0,DIRECTION(entity)==0);
                Bael_SpawnChargeEffect(*x,(s16)(*y+24),1,DIRECTION(entity)==0);
            }
            if (--entity->timer == 0xFF)
            {
                s32 direction = DIRECTION(entity);
                if (direction != 0)
                    direction = 1;
                DIRECTION(entity) = direction;
                /* Initialize the reset value before the state store: agbcc must
                 * retain this ordering to reproduce the original ROM. */
                {
                    s32 next = entity->stage + 1;
                    reset = 0;
                    entity->stage = next;
                }
                entity->timer = 32;
                {
                    s32 count = (s8)part->ext.bael.chargeCount;
                    if ((u32)count >= (u32)RandomNumberGenerator() % 5 + 1)
                    {
                        s32 zero = reset;
                        entity->subState = zero;
                        entity->stage = zero;
                        entity->accelX.whole = zero;
                        entity->velX.whole = zero;
                        entity->accelY.whole = zero;
                        entity->velY.whole = zero;
                        SPEED(entity) = (s32)SPEED(entity) / 2;
                        part->ext.bael.chargeCount = zero;
                        entity->timer = zero;
                    }
                }
            }
            break;
        }
        case 2:
        {
            s32 collision;
            work = entity->posX.whole;
            collision = Entity_Walk(entity,SPEED(entity),sBaelWalkProfile,6);
            Bael_RestoreXAtProbe(entity,work);
            if (collision & 0x40)
                SPEED(entity) = Bael_OffsetBySign(SPEED(entity),-0x666);
            else if (collision & 0x20)
                SPEED(entity) = Bael_OffsetBySign(SPEED(entity),0x666);
            ANGLE(entity) += (s32)SPEED(entity) / 64;
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,0,(work = 0),entity->affineIndex);
            if (--entity->timer == 0xFF)
            {
                entity->stage = work;
                part->ext.bael.chargeCount++;
            }
            break;
        }
    }
}

/* Bael death: shaking head, slowing rotation, smoke and final fragments. */
extern void Effect_SpawnRandom(s32 x,s32 y,s32 scaleX,s32 scaleY,s32 layer);
extern void Effect_SpawnAt(s32 x,s32 y,s32 layer);
extern void Bael_MoveAndRotate(struct EwramData_EntityData *entity,s32 mode);
extern s16 sBaelPositions[8][2];
#define SHAKE_X(e) (e->ext.wheelDeath.shake[0])
#define SHAKE_Y(e) (e->ext.wheelDeath.shake[1])

/**
 * @brief Shake the linked head, slow rotation, emit effects, delete the head, then free the affine slot after the final fragment burst.
 */
void Bael_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;
    switch (entity->subState)
    {
        case 0:
            entity->hit.flags |= 2;
            PlaySong(0x6E);
            part->drawSkip = 1;
            part->palette = part->basePalette;
            part->state++;
            entity->timer = 0x60;
            entity->palette = entity->basePalette;
            entity->subState++;
            if ((s32)SPEED(entity) >= 0)
                SPEED(entity) = 0x20000;
            else
                SPEED(entity) = -0x20000;
        case 1:
            if ((u8)((u32)entity->timer % 7) == 0)
                Effect_SpawnRandom(part->posX.whole,part->posY.whole,0x100000,0x100000,part->spriteLayer-1);
            if ((u8)((u32)entity->timer % 3) == 0)
            {
                SHAKE_X(part) = 1;
                SHAKE_Y(part) = 1;
                part->posX.part16.integer += 1;
                part->posY.part16.integer += SHAKE_Y(part);
            }
            else
            {
                s32 zero;
                part->posX.part16.integer -= SHAKE_X(part);
                part->posY.part16.integer -= SHAKE_Y(part);
                zero = 0;
                if (SHAKE_X(part) != 0)
                    SHAKE_X(part) = zero;
                if (SHAKE_Y(part) != 0)
                    SHAKE_Y(part) = zero;
            }
            if ((s32)SPEED(entity) >= 0)
            {
                SPEED(entity) -= 0x1C0;
                if ((s32)SPEED(entity) <= 0x6FFF)
                    SPEED(entity) = 0x7000;
            }
            else
            {
                SPEED(entity) += 0x1C0;
                if ((s32)SPEED(entity) > -0x7000)
                    SPEED(entity) = -0x7000;
            }
            ANGLE(entity) += (s32)SPEED(entity) / 64;
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,0,0,entity->affineIndex);
            Entity_Walk(entity,0,sBaelWalkProfile,2);
            part->posX.whole += entity->posX.whole-oldX;
            part->posY.whole += entity->posY.whole-oldY;
            if (--entity->timer == 0xFF)
            {
                part->deleteRequested = 1;
                entity->timer = 0x80;
                entity->subState++;
                entity->drawSkip = 1;
            }
            break;
        case 2:
            if (((u32)entity->timer % 2) == 0)
            {
                u32 timer;
                Effect_SpawnRandom(oldX,oldY,0x200000,0x200000,entity->spriteLayer-1);
                timer = entity->timer;
                if ((u8)(timer % 20) == 0 && timer > 28)
                    PlaySong(0x6E);
            }
            Bael_MoveAndRotate(entity,1);
            if (--entity->timer == 0xFF)
                entity->subState++;
            break;
        case 3:
        {
            s16 positions[8][2];
            s32 i;
            memcpy(positions,sBaelPositions,sizeof(positions));
            for (i=0; i<8; i++)
                Effect_SpawnAt(entity->posX.whole + (positions[i][0]<<16),entity->posY.whole + (positions[i][1]<<16) + 0x40000,entity->spriteLayer-1);
            PlaySong(0x6E);
            ObjAffine_Free(entity->affineIndex);
            entity->deleteRequested = 1;
            break;
        }
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BaelChargeEffect_Update = BuerChargeEffect_Update */

extern u8 sBaelBuerGfx[];
extern u8 sBaelBuerSpriteSheet[];

/**
 * @brief Use cached shared graphics and palette 6, run the selected animation with acceleration, then request deletion when animation finishes.
 */
void BaelChargeEffect_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
        {
            s32 gfx = ObjGfx_Find(sBaelBuerGfx);

            if (gfx < 0)
            {
                EntityDelete(entity);
                return;
            }
            entity->basePalette = 6;
            Sprite_Attach(entity, sBaelBuerSpriteSheet, gfx, 6);
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            Anim_StartIndexed(entity, entity->ext.wheel.effectAnim, 0, 0);
            entity->spriteLayer = 0x11;
            entity->scrollWithBg = 1;
            entity->state++;
        }
        case 1:
            EntityAccelerateAndMove(entity, 0x80000);
            break;
    }
    if (Anim_Update(entity) == 3)
        entity->deleteRequested = 1;
}

/* Bael collision motion, rotation, and update dispatcher. */
extern s32 Entity_IsOffsetOutsideRoom(struct EwramData_EntityData *entity,s32 x,s32 y);
extern void Bael_UpdateState4Rotation(struct EwramData_EntityData *entity);
extern void Bael_UpdateEntry(struct EwramData_EntityData *entity);
extern void Bael_UpdateActive(struct EwramData_EntityData *entity);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *entity);
extern void Hit_CheckWithPlayer(void *buffer,struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern s32 Collision_GetPushUp(s32 x,s32 y);
static inline s32 RoomX(s32 pos) { struct BgLayerState *bg = &gEwramData->bgInfo[1]; return pos + (s16)bg->xPos.part16.integer; }
static inline s32 RoomY(s32 pos) { struct BgLayerState *bg = &gEwramData->bgInfo[1]; return pos + (s16)bg->yPos.part16.integer; }

/**
 * @brief Walk, adjust signed speed from contacts, handle blocking probes, rotate and update the linked head facing.
 */
void Bael_MoveAndRotate(struct EwramData_EntityData *entity,s32 mode)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 oldX = entity->posX.whole;
    s32 options = 2;
    s32 collision;
    s32 sample;
    if (mode == 0)
        options = 6;
    collision = Entity_Walk(entity,entity->ext.wheel.speed,sBaelWalkProfile,options);
    sample = 24;
    if ((s32)entity->ext.wheel.speed < 0)
        sample = -24;
    if (Entity_IsOffsetOutsideRoom(entity,sample,0))
        entity->posX.whole = oldX;
    {
        s32 speed;
        s32 adjusted;
        if (collision & 0x40)
        {
            speed = entity->ext.wheel.speed;
            if (speed > 0)
                adjusted = speed - 0x666;
            else
                adjusted = speed + 0x666;
            goto set_speed;
        }
        else if (collision & 0x20)
        {
            speed = entity->ext.wheel.speed;
            if (speed > 0)
                adjusted = speed + 0x666;
            else
                adjusted = speed - 0x666;
            goto set_speed;
        }
        else if ((speed = entity->ext.wheel.speed) >= 0)
        {
            entity->ext.wheel.speed = speed - 0x20;
            if ((s32)entity->ext.wheel.speed <= 0x6FFF)
            {
                adjusted = 0x7000;
set_speed:
                entity->ext.wheel.speed = adjusted;
            }
        }
        else
        {
            entity->ext.wheel.speed = speed + 0x20;
            if ((s32)entity->ext.wheel.speed > -0x7000)
                entity->ext.wheel.speed = -0x7000;
        }
    }
    entity->ext.wheel.angle += (s32)entity->ext.wheel.speed / 64;
    if (collision & 3)
    {
        if (Collision_GetPushUp(RoomX((s16)entity->posX.part16.integer),RoomY((s16)entity->posY.part16.integer)+24))
            entity->ext.wheel.speed = -(s32)entity->ext.wheel.speed / 2;
        else
            entity->ext.wheel.speed = 0;
    }
    else if (mode == 0 && !(collision & 8))
        entity->ext.wheel.speed = -(s32)entity->ext.wheel.speed / 2;
    ObjAffine_Set(entity->ext.wheel.angle,0x10000,0x10000,0,0,entity->affineIndex);
    if ((s32)entity->ext.wheel.speed >= 0)
        part->flipX = 1;
    else
        part->flipX = 0;
}

/* Bael collision motion, rotation, and update dispatcher. */
void EnemyBaelUpdate(struct EwramData_EntityData *entity)
{
    u8 buffer[16];
    if (entity->state == 4)
    {
        Bael_UpdateState4Rotation(entity);
        return;
    }
    if (entity->state == 3)
    {
        Bael_UpdateDeath(entity);
        return;
    }
    Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity))
    {
        Hit_CheckWithPlayer(buffer,entity);
        return;
    }
    switch (entity->state)
    {
        case 1: Bael_UpdateEntry(entity); break;
        case 2: Bael_UpdateActive(entity); break;
        case 3: Bael_UpdateDeath(entity); break;
    }
    Hit_CheckWithPlayer(buffer,entity);
}
