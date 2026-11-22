#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern s32 Entity_GetTargetDirection(struct EwramData_EntityData *entity);
extern s16 Entity_GetTargetDistance(struct EwramData_EntityData *entity);
extern u8 sDiscArmorWalkProfile[];

static inline s32 WalkSpeed(s32 frame, s32 base) { return (frame << 13) + base; }
void DiscArmor_UpdateWalk(struct EwramData_EntityData *entity)
{
    u8 frame;
    s32 speed;
    s32 result;
    s32 attack;
    s32 distance;

    if (entity->subState == 0)
    {
        if (entity->unk_514.part16.spawnParam0 == 0)
        {
            entity->subState = 1;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        else
        {
            entity->subState = 2;
            Anim_StartIndexed(entity, 1, 0, 1);
        }
        entity->timer = 0;
    }
    switch (entity->subState)
    {
    case 2:
        frame = entity->animStep % 9;
        speed = WalkSpeed(frame, 0x1000);
        if (frame > 6)
            speed = 0;
        result = Entity_Walk(entity, speed, sDiscArmorWalkProfile, 20);
        if (*(u16 *)&entity->animStep == 0x11 && entity->timer++ > entity->unk_514.part16.spawnParam0)
        {
            entity->timer = 0;
            entity->flipX ^= 1;
        }
        if ((result & 0x1B) != 8)
            entity->flipX ^= 1;
        if (entity->animTimer != 0)
            return;
        if (entity->animStep != 0x11)
            return;
    case 1:
        if (Entity_GetTargetDistance(entity) > 0x4F)
        {
            if ((entity->flipX && (s8)Entity_GetTargetDirection(entity) == 1)
                || (!entity->flipX && (s8)Entity_GetTargetDirection(entity) == -1))
            {
                distance = Entity_GetTargetDistance(entity);
                attack = 1;
                if (distance <= 0x8B)
                    goto test;
            }
            attack = 0;
        test:
            if (!attack)
                return;
        }
        entity->state = 1;
        entity->stage = 0;
        entity->subState = 0;
    }
}

