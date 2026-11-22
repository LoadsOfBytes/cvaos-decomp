/* Legion corpse piece: wanders on the floor, then rises and swoops into a new shell part. */
#include "code_08009A0.h"
#include "legion_entity.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 Entity_MoveWithCollisionDefault(struct EwramData_EntityData *, s32, s32);
extern struct EwramData_EntityData *Legion_CreateCorpse(struct EwramData_EntityData *entity, s32 x, s32 y, s32 index);
extern void Effect_SpawnParticleAnim12(s32, s32, s32, s32, s32, s32, s32);
extern u8 sLegionAnimScript1[], sLegionAnimScript2[];

static inline s32 ABS(s32 x)
{
    if (x < 0)
        x = -x;
    return x;
}

#define TARGET_X LEGION_CORPSE_TARGET_X(entity)
#define TARGET_Y LEGION_CORPSE_TARGET_Y(entity)
#define FRAME LEGION_CORPSE_FRAME(entity)
#define BOB() entity->posY.whole += Sin(entity->velX.whole += 0x400) / 4;
#define APPROACH(V, T) \
    if (V > T) \
    { \
        if (ABS(V - T) <= 0x3FFF) \
            V = T; \
        else \
            V -= 0x4000; \
    } \
    else \
    { \
        if (ABS(V - T) <= 0x3FFF) \
            V = T; \
        else \
            V += 0x4000; \
    }

void LegionCorpse_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = LEGION_PART_OWNER(entity);
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 dx, dy;

    if (owner == NULL || owner->updateFunc == NULL || owner->unk_518.part8.enemyId != entity->unk_518.part8.enemyId)
        return;
    if (entity->state == 0)
    {
        if (!entity->flipX)
        {
            if (entity->posX.whole + bg->xPos.whole >= (s32)TARGET_Y)
            {
                TARGET_Y = TARGET_X - (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
                entity->velX.whole = -0x4000;
                entity->flipX = 1;
            }
        }
        else
        {
            if (entity->posX.whole + bg->xPos.whole <= (s32)TARGET_Y)
            {
                TARGET_Y = TARGET_X + (RandomNumberGenerator() & 0xFFFFF) + 0x80000;
                entity->velX.whole = 0x4000;
                entity->flipX = 0;
            }
        }
        Entity_MoveWithCollisionDefault(entity, 0x40000, -0x200000);
        Anim_Update(entity);
        if ((u8)LEGION_INTRO_STEP(owner) == FRAME + 1)
        {
            entity->state++;
            entity->subState = 0;
            entity->stage = 0;
        }
        return;
    }
    if (entity->state != 1)
        return;
    switch (entity->subState)
    {
    case 0:
        entity->subState++;
        Anim_Start(entity, (struct AnimScript *)(sLegionAnimScript1 + 0x1C), 2, 1);
        { s32 k = 0x600000; TARGET_Y = entity->posY.whole + bg->yPos.whole - (RandomNumberGenerator() % 0x300000 + k); }
        entity->posY.whole -= 0xD0000;
        entity->accelY.whole = -0x1000;
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        entity->velY.whole = 0;
        PlaySong(0x11E);
        break;
    case 1:
        EntityAccelerateAndMove(entity, 0x40000);
        if (entity->posY.whole + bg->yPos.whole > (s32)TARGET_Y)
            break;
        entity->accelY.whole = 0;
        entity->subState++;
        entity->timer = 0;
        break;
    case 2:
        EntityDampVelocityY(entity, 0xC);
        EntityAccelerateAndMove(entity, 0x40000);
        if (ABS(entity->velY.whole) > 0x8000)
            break;
        entity->subState++;
        LEGION_INTRO_ARRIVED(owner)++;
        entity->velX.whole = 0;
        break;
    case 3:
        BOB()
        if ((u8)LEGION_INTRO_STEP(owner) != 6)
            break;
        entity->subState++;
        entity->timer = FRAME * 20;
        break;
    case 4:
        BOB()
        if ((s8)--entity->timer <= 0)
        {
            Anim_Start(entity, (struct AnimScript *)sLegionAnimScript2, 2, 0);
            entity->subState++;
            PlaySong(0x11D);
        }
        if (entity->timer != 10)
            break;
        if (FRAME == 1)
            Legion_CreateCorpse(owner, 0x700000, 0x1000000, 10);
        if (FRAME == 3)
            Legion_CreateCorpse(owner, 0x1000000, 0x700000, 10);
        if (FRAME == 2)
            Legion_CreateCorpse(owner, 0x1900000, 0x1000000, 10);
        break;
    case 5:
        BOB()
        if (!entity->animDone)
            break;
        entity->subState++;
        TARGET_X = owner->posX.whole + (RandomNumberGenerator() & 0x7FFFFF) + ({ s32 b = bg->xPos.whole - 0x400000; b; });
        TARGET_Y = owner->posY.whole - (RandomNumberGenerator() & 0x3FFFFF) + bg->yPos.whole;
        entity->velX.whole = 0;
        entity->timer = 40;
        break;
    case 6:
        if (entity->timer > 19)
        {
            s32 t;
            dx = entity->posX.whole + bg->xPos.whole;
            dx = TARGET_X - dx;
            dy = entity->posY.whole + bg->yPos.whole;
            dy = TARGET_Y - dy;
            t = entity->timer;
            dx /= t - 1;
            dy /= t - 1;
            if (t == 20)
            {
                TARGET_X = owner->posX.whole + (RandomNumberGenerator() & 0xFFFFF) + ({ s32 b = bg->xPos.whole - 0x80000; b; });
                TARGET_Y = owner->posY.whole + (RandomNumberGenerator() & 0xFFFFF) + ({ s32 b = bg->yPos.whole - 0x80000; b; });
            }
        }
        else
        {
            dx = entity->posX.whole + bg->xPos.whole;
            dx = TARGET_X - dx;
            dy = entity->posY.whole + bg->yPos.whole;
            dy = TARGET_Y - dy;
            if (entity->timer != 1)
            {
                dx /= entity->timer - 1;
                dy /= entity->timer - 1;
            }
        }
        APPROACH(entity->velX.whole, dx)
        APPROACH(entity->velY.whole, dy)
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        if ((s8)--entity->timer > 0)
            break;
        if (FRAME == 0)
            LEGION_INTRO_ARRIVED(owner)++;
        Effect_SpawnParticleAnim12(entity->posX.whole, entity->posY.whole, 0, 0, entity->spriteLayer, 0, 6);
        PlaySong(0x11C);
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
}
