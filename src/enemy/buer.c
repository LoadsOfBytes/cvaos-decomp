/*
 * Buer: charge, death, charge effect and movement (the active state is in the previous, separately compiled file; the head and Buer_SpawnChargeEffect are in the next one: the charge code passes s32 coordinates to its u16 parameters).
 */
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include <string.h>
#include "code_08009A0.h"
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"

/* Buer accelerated charge, rotation, and sparks. */
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *entity);
extern s32 Entity_Walk(struct EwramData_EntityData *entity,s32 speed,void *profile,s32 flags);
extern void Buer_RestoreXAtProbe(struct EwramData_EntityData *entity,s32 work);
extern void Buer_SpawnChargeEffect(s32 x,s32 y,s32 index,s32 facing);
extern s32 Buer_OffsetBySign(s32 value,s32 step);
extern s16 sBuerWalkProfile[];
#define SPEED(e) (e->ext.wheel.speed)
#define ANGLE(e) (e->ext.wheel.angle)
#define DIRECTION(e) (e->ext.wheel.direction)

/**
 * @brief Accelerate toward the sampled side, rotate and emit paired effects; then run a 32-tick collision-adjusted movement phase.
 */
void Buer_UpdateCharge(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 work = entity->stage;
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
            work = entity->posX.whole;
            Entity_Walk(entity,SPEED(entity),sBuerWalkProfile,6);
            Buer_RestoreXAtProbe(entity,work);
            if (DIRECTION(entity))
            {
                SPEED(entity) += 0xCCC;
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
                SPEED(entity) -= 0xCCC;
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
                Buer_SpawnChargeEffect(initialX,(s16)(*y+24),0,DIRECTION(entity)==0);
                Buer_SpawnChargeEffect(*x,(s16)(*y+24),1,DIRECTION(entity)==0);
            }
            if (--entity->timer == 0xFF)
            {
                s32 direction = DIRECTION(entity);
                if (direction != 0)
                    direction = 1;
                DIRECTION(entity) = direction;
                entity->stage++;
                entity->timer = 32;
            }
            break;
        }
        case 2:
        {
            s32 collision;
            work = entity->posX.whole;
            collision = Entity_Walk(entity,SPEED(entity),sBuerWalkProfile,6);
            Buer_RestoreXAtProbe(entity,work);
            if (collision & 0x40)
                SPEED(entity) = Buer_OffsetBySign(SPEED(entity),-0x333);
            else if (collision & 0x20)
                SPEED(entity) = Buer_OffsetBySign(SPEED(entity),0x333);
            ANGLE(entity) += (s32)SPEED(entity) / 64;
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,0,(work = 0),entity->affineIndex);
            if (--entity->timer == 0xFF)
                entity->stage = work;
            break;
        }
    }
}

/* Buer death: shaking head, slowing rotation, smoke and final fragments. */
extern void Effect_SpawnRandom(s32 x,s32 y,s32 scaleX,s32 scaleY,s32 layer);
extern void Effect_SpawnAt(s32 x,s32 y,s32 layer);
extern void Buer_MoveAndRotate(struct EwramData_EntityData *entity,s32 mode);
extern s16 sBuerPositions[8][2];
#define SHAKE_X(e) (e->ext.wheelDeath.shake[0])
#define SHAKE_Y(e) (e->ext.wheelDeath.shake[1])

/**
 * @brief Shake the linked head, slow rotation, emit effects, delete the head, then free the affine slot after the final fragment burst.
 */
void Buer_UpdateDeath(struct EwramData_EntityData *entity)
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
                SPEED(entity) -= 0x380;
                if ((s32)SPEED(entity) <= 0x3FFF)
                    SPEED(entity) = 0x4000;
            }
            else
            {
                SPEED(entity) += 0x380;
                if ((s32)SPEED(entity) > -0x4000)
                    SPEED(entity) = -0x4000;
            }
            ANGLE(entity) += (s32)SPEED(entity) / 64;
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,0,0,entity->affineIndex);
            Entity_Walk(entity,0,sBuerWalkProfile,2);
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
            Buer_MoveAndRotate(entity,1);
            if (--entity->timer == 0xFF)
                entity->subState++;
            break;
        case 3:
        {
            s16 positions[8][2];
            s32 i;
            memcpy(positions,sBuerPositions,sizeof(positions));
            for (i=0; i<8; i++)
                Effect_SpawnAt(entity->posX.whole + (positions[i][0]<<16),entity->posY.whole + (positions[i][1]<<16) + 0x40000,entity->spriteLayer-1);
            PlaySong(0x6E);
            ObjAffine_Free(entity->affineIndex);
            entity->deleteRequested = 1;
            break;
        }
    }
}

/* BuerChargeEffect_Update. */
extern u8 sBaelBuerGfx[];
extern u8 sBaelBuerSpriteSheet[];

/**
 * @brief Use cached Buer graphics and palette 6, run the selected animation with acceleration, then request deletion when animation finishes.
 */
void BuerChargeEffect_Update(struct EwramData_EntityData *entity)
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

/* Buer collision motion, rotation, and update dispatcher. */
extern s32 Entity_IsOffsetOutsideRoom(struct EwramData_EntityData *entity,s32 x,s32 y);
extern void Buer_UpdateState4Rotation(struct EwramData_EntityData *entity);
extern void Buer_UpdateEntry(struct EwramData_EntityData *entity);
extern void Buer_UpdateActive(struct EwramData_EntityData *entity);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *entity);
extern void Hit_CheckWithPlayer(void *buffer,struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);

/**
 * @brief Walk with the selected collision mode, adjust speed from contact flags, rotate and update the linked head facing.
 */
void Buer_MoveAndRotate(struct EwramData_EntityData *entity,s32 mode)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 oldX = entity->posX.whole;
    s32 options = 2;
    s32 collision;
    s32 sample;
    if (mode == 0)
        options = 6;
    collision = Entity_Walk(entity,entity->ext.wheel.speed,sBuerWalkProfile,options);
    sample = 24;
    if ((s32)entity->ext.wheel.speed < 0)
        sample = -24;
    if (Entity_IsOffsetOutsideRoom(entity,sample,0))
        entity->posX.whole = oldX;
    if (collision & 0x40)
    {
        if ((s32)entity->ext.wheel.speed > 0)
            entity->ext.wheel.speed -= 0x333;
        else
            entity->ext.wheel.speed += 0x333;
    }
    else if (collision & 0x20)
    {
        if ((s32)entity->ext.wheel.speed > 0)
            entity->ext.wheel.speed += 0x333;
        else
            entity->ext.wheel.speed -= 0x333;
    }
    else
    {
        if ((s32)entity->ext.wheel.speed >= 0)
        {
            entity->ext.wheel.speed -= 0x40;
            if ((s32)entity->ext.wheel.speed <= 0x3FFF)
                entity->ext.wheel.speed = 0x4000;
        }
        else
        {
            entity->ext.wheel.speed += 0x40;
            if ((s32)entity->ext.wheel.speed > -0x4000)
                entity->ext.wheel.speed = -0x4000;
        }
    }
    entity->ext.wheel.angle += (s32)entity->ext.wheel.speed / 64;
    if ((collision & 3) || (mode == 0 && !(collision & 8)))
        entity->ext.wheel.speed = -(s32)entity->ext.wheel.speed / 2;
    ObjAffine_Set(entity->ext.wheel.angle,0x10000,0x10000,0,0,entity->affineIndex);
    if ((s32)entity->ext.wheel.speed >= 0)
        part->flipX = 1;
    else
        part->flipX = 0;
}

void EnemyBuerUpdate(struct EwramData_EntityData *entity)
{
    u8 buffer[16];
    if (entity->state == 4)
    {
        Buer_UpdateState4Rotation(entity);
        return;
    }
    if (entity->state == 3)
    {
        Buer_UpdateDeath(entity);
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
        case 1: Buer_UpdateEntry(entity); break;
        case 2: Buer_UpdateActive(entity); break;
        case 3: Buer_UpdateDeath(entity); break;
    }
    if (entity->state != 3)
        Hit_CheckWithPlayer(buffer,entity);
}
