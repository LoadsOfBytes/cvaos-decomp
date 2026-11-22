/* Attack and death states shared by Skull Millione, Cagnazzo and Lubicant. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/enemy_variant_b7e80.h"
extern void EntityMove(struct EwramData_EntityData *);
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);

/**
 * @brief Attack state (2): perform the variant dive and recovery sequence.
 * Behavior flag 2 adds a timed tracking phase before the dive.
 */
void SkullMillioneFamily_UpdateAttack(struct EwramData_EntityData *entity)
{
    const struct SkullMillioneFamilyVariant *info = &sSkullMillioneFamilyVariants[(u8)entity->unk_4F8.part8.unk_4F8];
    switch (entity->subState)
    {
    case 0:
        if (info->behaviorFlags & 2)
        {
            Anim_StartIndexed(entity, 3, 0, 1);
            entity->timer = 0x40;
            entity->subState = 5;
            break;
        }
        /* fall through */
    case 1:
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->velY.whole = -0x20000;
        entity->accelY.whole = 0x1000;
        entity->accelX.whole = -entity->velX.whole / 16;
        entity->subState = 2;
        entity->timer = 0x20;
        /* fall through */
    case 2:
        EntityMove(entity);
        if (entity->timer != 0)
        {
            entity->timer--;
            entity->velX.whole += entity->accelX.whole;
        }
        entity->velY.whole += entity->accelY.whole;
        if (entity->velY.whole > 0) entity->velY.whole = 0;
        if (entity->animStep == 4)
        {
            entity->subState = 3;
            entity->velY.whole = info->diveSpeed;
            entity->accelY.whole = -0x2000;
            PlaySong(0x87);
            if ((u8)entity->unk_4F8.part8.unk_4F8 == 1) PlaySong(0x1BA);
        }
        break;
    case 3:
        EntityAccelerateAndMoveY(entity, 0x30000);
        if (entity->animFlags & 4)
        {
            entity->accelY.whole = -0x2000;
            entity->subState = 4;
        }
        break;
    case 4:
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += entity->accelY.whole;
        if (entity->velY.whole < 0)
        {
            entity->velY.whole = 0;
            entity->state = 0;
            entity->stage = 0;
            entity->subState = 0;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        break;
    case 5:
        if (*(u16 *)&entity->animStep == 0) Entity_FaceTarget(entity);
        if (Target_GetX(entity) > entity->posX.whole) entity->accelX.whole = 0x3000;
        else entity->accelX.whole = -0x3000;
        if (Target_GetY(entity) - 0x680000 > entity->posY.whole)
            entity->accelY.whole = 0x1800;
        else
        {
            if (entity->velY.whole > -0x1000) entity->velY.whole = -0x1000;
            entity->accelY.whole = -0x3000;
        }
        EntityAccelerateAndMoveX(entity, 0x28000);
        EntityAccelerateAndMoveY(entity, 0x8000);
        if (entity->timer != 0) entity->timer--;
        else entity->subState = 1;
        break;
    }
}

/**
 * @brief Death state (3): emit particles, then fall with fragment effects.
 * Return nonzero outside the removal rectangle.
 */
s32 SkullMillioneFamily_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 x;
    struct EwramData_EntityData *particle;
    switch (entity->subState)
    {
    case 0:
        PlaySong(0x1BC);
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->subState = 1;
        entity->timer = 0x20;
        /* fall through */
    case 1:
        if (!(entity->timer & 0xF))
        {
            x = entity->posX.whole + (RandomSigned(8) << 16);
            particle = Effect_SpawnShared(x, entity->posY.whole + ((RandomSigned(0x10) - 0x10) << 16), 0xD, 8);
            if (particle != NULL)
            {
                particle->flipX = RandomNumberGenerator() & 1;
                particle->spriteLayer = 4;
            }
        }
        if (entity->timer != 0) entity->timer--;
        else
        {
            entity->velY.whole = -0x8000;
            entity->accelY.whole = 0x1000;
            entity->subState = 2;
            entity->timer = 0x32;
        }
        break;
    case 2:
        EntityAccelerateAndMoveY(entity, 0x80000);
        Enemy_UpdateFragmentDeath(entity);
        break;
    }
    if (!Entity_IsInRect(entity, -0x80, -0x40, 0x1F0, 0x120)) return 1;
    return 0;
}
