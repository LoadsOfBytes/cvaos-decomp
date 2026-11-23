#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *);
extern s32 Entity_GetTargetDistanceY(struct EwramData_EntityData *);
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern s32 Entity_IsTargetInCenteredBox(struct EwramData_EntityData *, s32, s32);
extern s32 Entity_Walk(struct EwramData_EntityData *, s32, void *, s32);

/**
 * @brief Face the target, choose short walking intervals and select attack or idle states from proximity and random checks.
 */
void Skeleton_UpdateCombatMovement(struct EwramData_EntityData *entity)
{
    s32 state = entity->subState;
    s32 d0;
    s32 d;
    s32 d2;
    s32 anim;
    u32 r;

    switch (state)
    {
    case 0:
        d0 = (s16)Entity_GetTargetDistanceY(entity);
        if (d0 < 0)
            d0 = -d0;
        if (d0 > 0x40)
        {
            d = (s16)Entity_GetTargetDistanceX(entity);
            if (d < 0)
                d = -d;
            if (d > 0x200)
            {
                entity->state = 0;
                entity->stage = 0;
                entity->subState = 0;
                break;
            }
        }
        entity->ext.skeleton.cooldown = RandomNumberGenerator() % 32;
        if ((entity->ext.skeleton.flags & 0xA) == 0xA)
            entity->ext.skeleton.cooldown = 0x40;
        entity->ext.skeleton.cooldown = 1;
        entity->subState++;
        entity->stage = 0;
    case 1:
        Entity_FaceTarget(entity);
        entity->ext.skeleton.walkSpeed = 0x8000;
        if (!entity->flipX)
            entity->velX.whole = -entity->velX.whole;
        if (Entity_IsTargetInCenteredBox(entity, RandomNumberGenerator() % 64 + 0x80, 0x80))
            entity->ext.skeleton.walkSpeed = -entity->ext.skeleton.walkSpeed;
        anim = (entity->ext.skeleton.flags & 8) ? 2 : 1;
        if (entity->animScriptIndex != anim)
            Anim_StartIndexed(entity, anim, 0, 1);
        if ((entity->ext.skeleton.flags & 0xA) == 0xA)
        {
            entity->velX.whole = 0;
            entity->animFlags |= 2;
        }
        else
        {
            entity->animFlags &= ~2;
        }
        entity->timer = RandomNumberGenerator() % 8 + 1;
        entity->subState++;
        entity->stage = 0;
    case 2:
        if (entity->ext.skeleton.cooldown)
            entity->ext.skeleton.cooldown--;
        if ((entity->ext.skeleton.flags & 6) && !(RandomNumberGenerator() % 16))
            entity->ext.skeleton.flags |= 0x20;
        else
            entity->ext.skeleton.flags &= ~0x20;
        entity->ext.skeleton.walkSpeed -= entity->ext.skeleton.walkSpeed >> 4;
        if ((entity->ext.skeleton.flags & 0xA) == 0xA && entity->ext.skeleton.cooldown == 0)
        {
            r = RandomNumberGenerator() & 3;
            if (r)
            {
                entity->state = 4;
                entity->stage = 0;
                entity->subState = 0;
            }
            else
            {
                entity->state = 2;
                entity->stage = r;
                entity->subState = r;
            }
            break;
        }
        if (entity->timer == 0)
        {
            if (entity->ext.skeleton.cooldown == 0 && Entity_IsTargetInCenteredBox(entity, 0xC4, 0x80))
            {
                if (entity->ext.skeleton.flags & 6)
                {
                    entity->state = 4;
                    entity->stage = 0;
                    entity->subState = 0;
                }
                else
                {
                    entity->state = 3;
                    entity->stage = 0;
                    entity->subState = 0;
                }
                entity->velX.whole = 0;
                break;
            }
            entity->ext.skeleton.throwUrge++;
            if (RandomNumberGenerator() % 40 < entity->ext.skeleton.throwUrge)
            {
                d2 = (s16)Entity_GetTargetDistanceX(entity);
                if (d2 < 0)
                    d2 = -d2;
                if (d2 > 0x30)
                {
                    entity->ext.skeleton.throwUrge = 0;
                    if (RandomNumberGenerator() % 50 <= 14)
                    {
                        entity->state = 2;
                        entity->stage = 0;
                        entity->subState = 0;
                    }
                    break;
                }
                entity->ext.skeleton.throwUrge >>= 1;
            }
            entity->subState--;
            entity->stage = 0;
        }
        else
        {
            entity->timer--;
        }
        Entity_Walk(entity, entity->ext.skeleton.walkSpeed, (void *)0x08528948, 0xC);
        break;
    }
}
