#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern void AluraUne_SpawnThorn(struct EwramData_EntityData *, s32, s32, s32);
extern void ScreenShake_RequestOffset(u8 priority, s8 offsetX, s8 offsetY);
extern s32 Sound_PlayTracked(s32 song);
extern s32 Sound_PlayTrackedIfAbsent(s32 song);
extern s32 Sound_StopTracked(s32 song);

/**
 * @brief 7C150 | Boss AI (subState), always facing the player: wakes on the
 * player box 0xB4 x 0x60 (first time straight to the entrance, state 10),
 * then picks at random between an animation 1 / 3 / 4 sequence with a
 * random 60-119 frame wait (unk_505 cuts it short), attack mode
 * (state = 1) or the roar (state 0xB): states 10-13 spawn eight debris
 * pieces (AluraUne_SpawnThorn), shake the screen (ScreenShake_RequestOffset) and play sound
 * 0x105 (Sound_PlayTracked / Sound_PlayTrackedIfAbsent / Sound_StopTracked).
 */
void AluraUne_UpdateIdle(struct EwramData_EntityData *entity)
{
    u32 r;
    s32 i;

    entity->flipX = Entity_IsTargetRight(entity);
    switch (entity->subState)
    {
    case 0:
        if (Target_IsInRect(entity->posX.part16.integer - 0x5A, entity->posY.part16.integer - 0x30, 0xB4, 0x60) || Enemy_IsDamaged(entity))
        {
            if (!*(s8 *)&entity->ext.aluraUne.awake)
            {
                entity->ext.aluraUne.awake = 1;
                entity->subState = 10;
                break;
            }
            r = RandomNumberGenerator() % 3;
            if (r == 0)
            {
                entity->subState++;
                Anim_StartIndexed(entity, 1, 0, 0);
            }
            else if (r == 1)
            {
                entity->state = r;
                entity->subState = 0;
            }
            else
            {
                entity->state = 1;
                entity->subState = 10;
            }
        }
        break;
    case 1:
        if (entity->animDone)
        {
            entity->subState++;
            Anim_StartIndexed(entity, 3, 0, 0);
        }
        break;
    case 2:
        if (entity->animDone)
        {
            entity->subState++;
            entity->timer = RandomNumberGenerator() % 60 + 60;
            Anim_StartIndexed(entity, 4, 0, 1);
        }
        break;
    case 3:
        if (*(s8 *)&entity->ext.aluraUne.hit)
        {
            entity->timer = 0;
            entity->ext.aluraUne.hit = 0;
        }
        if ((s8)entity->timer-- > 0)
            break;
        r = RandomNumberGenerator() % 3;
        if (r == 0)
        {
            entity->subState++;
            Anim_StartIndexed(entity, 0xB, 0, 0);
        }
        else if (r == 1)
        {
            entity->state = r;
            entity->subState = r;
        }
        else
        {
            entity->state = 1;
            entity->subState = 0xB;
            entity->animDone = 1;
        }
        break;
    case 4:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 10, 0, 0);
            entity->subState++;
            entity->timer = 0;
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
        if (entity->timer++ > 0x3B)
        {
            entity->subState = 0;
            entity->timer = 0;
        }
        break;
    case 10:
        entity->subState++;
        entity->timer = 0;
        for (i = 0; i <= 3; i++)
        {
            AluraUne_SpawnThorn(entity, (RandomNumberGenerator() & 0xF) + i * 16, 0, 0x28);
            AluraUne_SpawnThorn(entity, -(RandomNumberGenerator() & 0xF) - i * 16, 0, 0x28);
        }
        break;
    case 11:
        if (!(entity->timer & 7))
            ScreenShake_RequestOffset(0, 0, 1);
        else
            ScreenShake_RequestOffset(0, 0, 0);
        entity->subState++;
        entity->timer = 0;
        Anim_StartIndexed(entity, 1, 0, 0);
        Sound_PlayTracked(0x105);
        break;
    case 12:
        entity->timer++;
        if ((entity->timer & 3) == 0)
        {
            s32 a = (RandomNumberGenerator() & 1) + 1;
            s32 b = (RandomNumberGenerator() & 1) + 1;
            ScreenShake_RequestOffset(0, a, b);
            Sound_PlayTrackedIfAbsent(0x105);
        }
        else
        {
            ScreenShake_RequestOffset(0, 0, 0);
        }
        if (entity->animDone)
        {
            entity->subState++;
            entity->timer = 0;
            Anim_StartIndexed(entity, 2, 0, 1);
        }
        break;
    case 13:
        if ((entity->timer & 3) == 0)
        {
            s32 a = (RandomNumberGenerator() & 1) + 1;
            s32 b = (RandomNumberGenerator() & 1) + 1;
            ScreenShake_RequestOffset(0, a, b);
            Sound_PlayTrackedIfAbsent(0x105);
        }
        else
        {
            ScreenShake_RequestOffset(0, 0, 0);
        }
        if (entity->timer++ > 0x1D)
        {
            entity->state = 0;
            entity->subState = 2;
            Anim_StartIndexed(entity, 3, 0, 0);
            Sound_StopTracked(0x105);
        }
        break;
    }
}
