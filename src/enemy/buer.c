/*
 * Buer: create, head, active / charge / death states, charge effect and movement.
 * Built without -f2003-patch (Buer_CreateHead / Buer_UpdateActive need it off; the rest compiles the same either way).
 */
#include "core.h"
#include "graphics.h"
#include "gameplay.h"
#include "structs/ewram.h"
#include "sound.h"
#include <string.h>
#include "player_core.h"
#include "bael.h"
#include "buer.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "entity_inline.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Buer_SpawnChargeEffect(s32 x,s32 y,s32 index,s32 facing);
extern s32 Entity_IsOffsetOutsideRoom(struct EwramData_EntityData *entity,s32 x,s32 y);

void Buer_CreateHead(struct EwramData_EntityData *);
void Buer_UpdateCharge(struct EwramData_EntityData *entity);

/* Buer: load graphics, create its linked parts and set initial collision
 * and movement state. */
void EnemyBuerCreate(struct EwramData_EntityData *entity)
{
    u8 *palette = sBaelBuerPalette;
    u8 *sprites = sBaelBuerSpriteSheet;
    u8 *graphics = sBaelBuerGfx;
    if (!Entity_LoadGraphics(entity, graphics, palette, 0, sprites))
    {
        EntityDelete(entity);
        return;
    }
    entity->spriteLayer = 20;
    Enemy_ResetFromTemplate(entity);
    entity->animFrame = 0;
    entity->scrollWithBg = 1;
    entity->ext.wheel.speed = 0x8000;
    entity->accelY.whole = 0x4000;
    if (entity->spawnParam0 == 0) Entity_SetStateAndStage(entity, 0, 1);
    else Entity_SetStateAndStage(entity, 0, 2);
    Buer_CreateHead(entity);
    Buer_AllocateAffine(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        Entity_SetStateAndStage(entity, 0, 4);
    }
    else
    {
        Hit_Enable(entity, 8, Buer_DamagePlayerOnContact, Buer_OnHit);
        HitBox_SetAttackRaw(entity, 0x2828ECEC);
        HitBox_SetBodyRaw(entity, 0x2828ECEC);
        Entity_SnapToGround(entity);
        entity->posY.part16.integer -= 24;
    }
}

/* Buer_CreateHead. */

/**
 * @brief Create the linked frame-1 head sharing graphics and palette; store mutual slot indices and face it according to movement speed.
 */
void Buer_CreateHead(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = Entity_CreateObject(BuerHead_Update);

    if (child != NULL)
    {
        ENTITY_PARENT_INDEX(child) = ENTITY_INDEX(entity);
        ENTITY_PARENT_INDEX(entity) = ENTITY_INDEX(child);
        child->spriteLayer = entity->spriteLayer - 1;
        child->basePalette = entity->basePalette;
        child->flipX = entity->flipX;
        Sprite_Attach(child, sBaelBuerSpriteSheet, Sprite_GetTileSlot(entity), child->basePalette);
        child->animFrame = 1;
        child->scrollWithBg = 1;
        child->drawFunc = Sprite_DrawEntity;
        child->posX.whole = entity->posX.whole;
        child->posY.whole = entity->posY.whole;
        if ((s32)entity->ext.wheel.speed >= 0)
            child->flipX = 1;
        else
            child->flipX = 0;
    }
}

/* Bael/Alastor part behaviour 0x080CBAEC; only matches without -f2003-patch. */

#define ABS(x) ((x) < 0 ? -(x) : (x))

/**
 * @brief Move, choose a charge from target proximity and linked-head facing, or periodically reverse facing and reconsider movement.
 */
void Buer_UpdateActive(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *other = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    // zero and timer: explicit temporaries for the original register use
    s32 zero;
    s32 timer;

    if (Buer_CheckCleanup(entity))
        return;
    switch (entity->subState)
    {
        case 0:
            Buer_MoveAndRotate(entity, 0);
            if (Target_IsNearEntity(entity, 0xC0, 0x30)
                && ((!other->flipX && (s8)Entity_GetTargetDirection(entity) == -1)
                    || (other->flipX && (s8)Entity_GetTargetDirection(entity) == 1)))
            {
                entity->subState = 1;
                entity->stage = 0;
            }
            timer = entity->timer + 1;
            zero = 0;
            entity->timer = timer;
            if ((u8)timer == 0xFF)
            {
                entity->timer = zero;
                if (!(RandomNumberGenerator() % 2))
                    entity->subState = 2;
                goto restart;
            }
            break;
        case 1:
            Buer_UpdateCharge(entity);
            break;
        case 2:
            switch (entity->stage)
            {
                case 0:
                    entity->timer = RandomNumberGenerator() % 32 + 32;
                    entity->stage++;
                case 1:
                    other->flipX = 1 - other->flipX;
                    entity->stage++;
                case 2:
                    if (Target_IsNearEntity(entity, 0x60, 0x30))
                    {
                        entity->subState = 1;
                        entity->stage = 0;
                    }
                    timer = entity->timer - 1;
                    zero = 0;
                    entity->timer = timer;
                    if ((u8)timer == 0xFF)
                    {
                        if (RandomNumberGenerator() % 2)
                            goto restart;
                        entity->subState = zero;
                        entity->stage = zero;
                        entity->timer = zero;
                        // unk_4FC is read as signed here
                        if (other->flipX)
                            *(s32 *)&entity->ext.wheel.speed = ABS(*(s32 *)&entity->ext.wheel.speed);
                        else
                            *(s32 *)&entity->ext.wheel.speed = -ABS(*(s32 *)&entity->ext.wheel.speed);
                    }
                    break;
            }
            break;
    }
    return;
restart:
    entity->stage = zero;
}

/* Buer accelerated charge, rotation, and sparks. */
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
            entity->drawFunc = Sprite_DrawEntity;
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
        Hit_CheckWithPlayer((struct HitResult *)buffer,entity);
        return;
    }
    switch (entity->state)
    {
        case 1: Buer_UpdateEntry(entity); break;
        case 2: Buer_UpdateActive(entity); break;
        case 3: Buer_UpdateDeath(entity); break;
    }
    if (entity->state != 3)
        Hit_CheckWithPlayer((struct HitResult *)buffer,entity);
}
