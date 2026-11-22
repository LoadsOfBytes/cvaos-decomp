/* AluraUne_UpdateAttack: enemy behaviour state machine (14 states). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern void AluraUne_SpawnThorn(struct EwramData_EntityData *, s32, s32, s32);
extern void AluraUne_SpawnRootWave(struct EwramData_EntityData *parent, s32 index, s32 facing);
extern void ScreenShake_RequestOffset(u8 priority, s8 param_1, s8 param_2);
extern s32 Sound_PlayTracked(s32 song);
extern s32 Sound_PlayTrackedIfAbsent(s32 song);
extern s32 Sound_StopTracked(s32 song);

/**
 * @brief Attack state machine (state 1): animation 1, then rows of thorns at
 * random gaps on both sides (AluraUne_SpawnThorn) or two root waves along
 * the floor in both directions (AluraUne_SpawnRootWave); then back to
 * state 0.
 */
void AluraUne_UpdateAttack(struct EwramData_EntityData *entity)
{
    u32 r;
    u32 m;
    s32 i;
    s32 skip1, skip2;

    entity->flipX = Entity_IsTargetRight(entity);
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->subState++;
        break;
    case 1:
        if (entity->animDone)
        {
            skip1 = RandomNumberGenerator() % 6;
            skip2 = RandomNumberGenerator() % 6;
            for (i = 0; i <= 6; i++)
            {
                if (i != skip1 && i != skip2)
                    AluraUne_SpawnThorn(entity, i * 16 + 0x28, -1, 0x6E);
            }
            skip1 = RandomNumberGenerator() % 6;
            skip2 = RandomNumberGenerator() % 6;
            for (i = 0; i <= 6; i++)
            {
                if (i != skip1 && i != skip2)
                    AluraUne_SpawnThorn(entity, -(i * 16 + 0x28), -1, 0x6E);
            }
            entity->unk_504.part8.unk_505 = 0;
            entity->subState++;
            entity->timer = 0;
            if (entity->animScriptIndex != 4)
                Anim_StartIndexed(entity, 3, 0, 0);
            else
                entity->subState = entity->animScriptIndex;
            Sound_PlayTracked(0x105);
        }
        break;
    case 2:
        entity->timer++;
        if ((entity->timer & 3) == 0)
            Sound_PlayTrackedIfAbsent(0x105);
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 4, 0, 1);
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 3:
        entity->subState++;
    case 4:
        if ((entity->timer & 3) == 0)
            Sound_PlayTrackedIfAbsent(0x105);
        if (entity->timer > 0x1D)
        {
            if (entity->timer <= 0x5A)
            {
                if (!(entity->timer & 7))
                    ScreenShake_RequestOffset(0, 0, 1);
                else
                    ScreenShake_RequestOffset(0, 0, 0);
            }
            else
            {
                if (entity->timer == 0x6E)
                    Anim_StartIndexed(entity, 2, 0, 1);
                if ((entity->timer & 3) == 0)
                {
                    s32 a = (RandomNumberGenerator() & 1) + 1;
                    s32 b = (RandomNumberGenerator() & 1) + 1;
                    ScreenShake_RequestOffset(0, a, b);
                }
                else
                {
                    ScreenShake_RequestOffset(0, 0, 0);
                }
            }
        }
        if (entity->timer++ <= 0xFC)
            break;
        Sound_StopTracked(0x105);
        if (EntityIsOutsideScreen(entity, 4))
        {
            entity->state = 0;
            entity->subState = 0;
            Anim_StartIndexed(entity, 0, 0, 1);
            break;
        }
        r = RandomNumberGenerator() % 3;
        if (r == 0)
        {
            entity->subState++;
            entity->timer = r;
            Anim_StartIndexed(entity, 10, 0, 0);
        }
        else if (r == 1)
        {
            entity->state = 0;
            entity->subState = 2;
            Anim_StartIndexed(entity, 3, 0, 0);
        }
        else
        {
            entity->animDone = 1;
            entity->subState = 0xB;
        }
        break;
    case 5:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 6:
        if (entity->timer++ > 0x59)
        {
            entity->state = 0;
            entity->subState = 0;
            entity->timer = 0;
        }
        break;
    case 10:
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->subState++;
        break;
    case 11:
        if (entity->animDone)
        {
            entity->unk_504.part8.unk_505 = 0;
            entity->subState++;
            entity->timer = 0;
            Anim_StartIndexed(entity, 5, 0, 0);
        }
        break;
    case 12:
        if (entity->animDone)
        {
            entity->subState++;
            entity->timer = 0;
            Anim_StartIndexed(entity, 6, 0, 1);
            break;
        }
        m = 0xFFFF;
        if ((*(u16 *)&entity->animStep & m) == 3)
        {
            AluraUne_SpawnRootWave(entity, 0, entity->flipX);
            AluraUne_SpawnRootWave(entity, 0, ~(*(ENTITY_RENDER_FLAGS_ADDRESS(entity)) >> 6) & 1);
        }
        m &= *(u16 *)&entity->animStep;
        if (m == 4)
        {
            AluraUne_SpawnRootWave(entity, 1, entity->flipX);
            AluraUne_SpawnRootWave(entity, 1, ~(*(ENTITY_RENDER_FLAGS_ADDRESS(entity)) >> 6) & 1);
        }
        break;
    case 13:
        if (entity->timer++ > 0xEF)
        {
            if (EntityIsOutsideScreen(entity, 4))
            {
                entity->state = 0;
                entity->subState = 0;
                Anim_StartIndexed(entity, 0, 0, 1);
                break;
            }
            r = RandomNumberGenerator() % 3;
            if (r == 0)
            {
                entity->subState = 5;
                entity->timer = 0;
                Anim_StartIndexed(entity, 10, 0, 0);
            }
            else if (r == 1)
            {
                entity->state = 0;
                entity->subState = 2;
                Anim_StartIndexed(entity, 3, 0, 0);
            }
            else
            {
                Anim_StartIndexed(entity, 0xB, 0, 0);
                entity->subState = 1;
            }
        }
        break;
    }
}
