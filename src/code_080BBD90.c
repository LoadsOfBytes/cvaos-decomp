
/* Cockatrice/Basilisk walk 0x080BBD90. This unit omits -f2003-patch to preserve the signed -1 test.
 * Local register bindings preserve the frame-speed and timer copies.
 * Matching does not establish original source spelling. */
#include "code/code_08039340.h"

extern s32 Entity_Walk(struct EwramData_EntityData *entity, s32 speed, void *profile, s32 flags);
extern s32 Entity_GetTargetDirection(struct EwramData_EntityData *entity);
extern s16 Entity_GetTargetDistance(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *updateFunc);
extern void Entity_FaceTarget(struct EwramData_EntityData *entity);
extern void CockatriceFamilyAttack_Update(struct EwramData_EntityData *entity);
extern u8 sCockatriceFamilyWalkProfile[];

/**
 * @brief Attack state (1): walk and face the target, start animation 1
 * when the direction, distance and player flag tests allow it, then spawn
 * the attack effect at animation step 10. Recover for 32 updates.
 */
void CockatriceFamily_UpdateAttack(struct EwramData_EntityData *entity)
{
    u8 frame;
    s32 speed;
    s32 attack;
    s32 distance;
    struct EwramData_EntityData *child;

    switch (entity->subState)
    {
        case 0:
            entity->subState = 1;
        case 1:
            frame = (entity->animStep + 1) % 8;
            {
                register s32 scaled asm("r1") = frame * 0x4000; /* FAKEMATCH: pinned register */
                speed = scaled + 0x1000;
            }
            if (frame > 4)
                speed = 0;
            Entity_Walk(entity, speed, sCockatriceFamilyWalkProfile, 20);
            // animStep and animTimer read together
            if (*(u16 *)&entity->animStep == 0)
            {
                if ((entity->flipX && (s8)Entity_GetTargetDirection(entity) == 1)
                    || (!entity->flipX && (s8)Entity_GetTargetDirection(entity) == -1))
                {
                    distance = Entity_GetTargetDistance(entity);
                    attack = 1;
                    if (distance <= 119)
                        goto test;
                }
                attack = 0;
            test:
                if (attack && !(gEwramData->playerStatus.statusEffects & 4))
                {
                    entity->subState = 2;
                    Anim_StartIndexed(entity, 1, 0, 0);
                }
                Entity_FaceTarget(entity);
            }
            break;
        case 2:
            if (*(u16 *)&entity->animStep == 10 && (child = Entity_CreateObject((u32 *)CockatriceFamilyAttack_Update)) != NULL)
            {
                child->unk_4F8.unk_4F8_entity = entity;
                child->scrollWithBg = 1;
                child->attack = entity->attack - (entity->attack >> 3);
            }
            if (entity->animFlags & 4)
            {
                entity->timer = 0x20;
                entity->subState = 3;
            }
            break;
        case 3:
        {
            u8 raw = entity->timer;
            s8 timer = raw;
            if (timer != 0)
            {
                entity->timer = raw - 1;
                break;
            }
            entity->state = timer;
            entity->stage = timer;
            entity->subState = timer;
            Anim_StartIndexed(entity, 0, 0, 1);
            break;
        }
    }
}
