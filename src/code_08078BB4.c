/* Werejaguar_UpdateAttack, decompiled from the original assembly: hopping enemy AI
 * (ids 0x4B / 0x54), sibling of Werejaguar_UpdateWalk. The shared failure tail
 * (fail:) and the state 10 attack start sit where the original keeps their
 * cross-jumped copies; the hit test results use a function-level d. */
#include "code_08009A0.h"
#include "werejaguar_entity.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s16 Target_GetPixelX(struct EwramData_EntityData *entity);
extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);

static inline s32 Dist(struct EwramData_EntityData *entity)
{
    s32 d = Target_GetPixelX(entity) - (s16)entity->posX.part16.integer;
    if (d < 0)
        d = -d;
    return d;
}

#define HOP_FLAG (entity->unk_4F8.bytes[3])
#define ROLL (entity->unk_4F8.bytes[2])

void Werejaguar_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 r;
    s32 speed;
    s32 zero;
    s32 d;

    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        speed = 0x20000;
        d = Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 8);
        if (!(d & 4))
            break;
        if (entity->timer++ > 0x1D || (Player_GetFocusedActionFlags() & 0x360))
        {
            entity->timer = 0;
            entity->subState++;
            WEREJAGUAR_CONTACT_HIT(entity) = 0;
            WEREJAGUAR_MANUAL_ANIM(entity) = 1;
            WEREJAGUAR_UNUSED_FLAG(entity) = 0;
            entity->animFrame = 0x10;
            Entity_UpdateBoxes(entity);
            entity->accelX.whole = 0;
            entity->velY.whole = 0x40000;
            entity->accelY.whole = 0x2800;
            if (entity->flipX)
                entity->velX.whole = speed;
            else
                entity->velX.whole = -0x20000;
            break;
        }
        if (entity->unk_518.part8.enemyId != 0x54)
            break;
        if (Target_IsInRect(entity->posX.part16.integer - 0x3C, entity->posY.part16.integer - 0x78, 0x78, 0x6E))
        {
            Anim_StartIndexed(entity, 4, 0, 0);
            entity->subState = 10;
        }
        break;
    case 1:
        WEREJAGUAR_MANUAL_ANIM(entity) = 1;
        r = Entity_MoveWithCollision(entity, 0x40000, -0x200000, 0x80000, 8) & 4;
        if (!r)
            goto fail;
        if (entity->timer++ <= 0x13)
        {
            if (entity->timer <= 3)
                break;
            if (Dist(entity) > 0x14)
                break;
        }
        entity->subState = 2;
        entity->velX.whole = 0;
        entity->animFrame = 0;
        entity->timer = 0;
        Entity_UpdateBoxes(entity);
        break;
    case 2:
        if (entity->timer++ <= 5)
            break;
        r = Target_IsInRect(entity->posX.part16.integer - 0x14, entity->posY.part16.integer - 0x28, 0x28, 0x50);
        if (r != 0)
        {
            if (Dist(entity) <= 0xC)
                goto close;
            entity->subState = 3;
            if (entity->unk_518.part8.enemyId == 0x4B)
            {
                if (RandomNumberGenerator() % 3)
                    Anim_StartIndexed(entity, 3, 0, 0);
                else if (!(RandomNumberGenerator() & 1))
                    Anim_StartIndexed(entity, 1, 0, 0);
                else
                    Anim_StartIndexed(entity, 2, 0, 0);
            }
            else
                Anim_StartIndexed(entity, 3, 0, 0);
            WEREJAGUAR_MANUAL_ANIM(entity) = 0;
            break;
        }
        WEREJAGUAR_MANUAL_ANIM(entity) = r;
        Anim_StartIndexed(entity, 6, 0, 1);
        entity->state = r;
        entity->subState = 1;
        break;
    case 3:
        if (Dist(entity) <= 0xC)
        {
        close:
            entity->timer = 0;
            entity->subState = 6;
            WEREJAGUAR_MANUAL_ANIM(entity) = 1;
            entity->animFrame = 0x10;
            Entity_UpdateBoxes(entity);
            entity->accelX.whole = 0;
            entity->velY.whole = 0x40000;
            entity->accelY.whole = 0x2800;
            if (entity->flipX)
                entity->velX.whole = -0x20000;
            else
                entity->velX.whole = 0x20000;
            break;
        }
        if (entity->animDone)
        {
            s32 hop = HOP_FLAG;
            if (hop != 0)
            {
                if (entity->unk_518.part8.enemyId == 0x4B)
                {
                    entity->subState = 5;
                    entity->timer = 0;
                    Anim_StartIndexed(entity, 0, 0, 0);
                    break;
                }
                entity->timer = 0;
                entity->subState = 0xD;
                WEREJAGUAR_MANUAL_ANIM(entity) = 1;
                entity->animFrame = 0x10;
                Entity_UpdateBoxes(entity);
                entity->accelX.whole = 0;
                {
                    s32 v = 0x20000;
                    entity->velY.whole = v;
                    entity->accelY.whole = 0x2800;
                    if (entity->flipX)
                        entity->velX.whole = -0x20000;
                    else
                        entity->velX.whole = v;
                }
                break;
            }
            Anim_StartIndexed(entity, 6, 0, 1);
            entity->state = hop;
            entity->subState = 1;
            WEREJAGUAR_MANUAL_ANIM(entity) = hop;
            break;
        }
        WEREJAGUAR_CONTACT_KIND(entity) = 1;
        if (entity->animTimer == 0 && (entity->animStep == 1 || entity->animStep == 3 || entity->animStep == 5))
            PlaySong(0x85);
        break;
    case 4:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 6, 0, 1);
            entity->state = 0;
            entity->subState = 1;
            break;
        }
        if (entity->animStep == 5)
            WEREJAGUAR_CONTACT_KIND(entity) = 2;
        else
            WEREJAGUAR_CONTACT_KIND(entity) = 1;
        if (entity->animTimer == 0 && (entity->animStep == 1 || entity->animStep == 3 || entity->animStep == 5))
            PlaySong(0x85);
        break;
    case 5:
        if (entity->timer++ > 7)
        {
            if (entity->unk_518.part8.enemyId == 0x4B)
            {
                if (ROLL == 0 && !(RandomNumberGenerator() & 3))
                {
                    ROLL = 1;
                    if (RandomNumberGenerator() % 3)
                        Anim_StartIndexed(entity, 3, 0, 0);
                    else if (!(RandomNumberGenerator() & 1))
                        Anim_StartIndexed(entity, 1, 0, 0);
                    else
                        Anim_StartIndexed(entity, 2, 0, 0);
                    entity->subState = 3;
                    break;
                }
                if (RandomNumberGenerator() & 1)
                    Anim_StartIndexed(entity, 1, 0, 1);
                else
                    Anim_StartIndexed(entity, 2, 0, 1);
                entity->subState = 4;
                break;
            }
            Anim_StartIndexed(entity, 4, 0, 0);
            entity->subState = 10;
            break;
        }
        WEREJAGUAR_CONTACT_KIND(entity) = 1;
        break;
    case 6:
        zero = 0;
        WEREJAGUAR_MANUAL_ANIM(entity) = 1;
        r = Entity_MoveWithCollision(entity, 0x40000, -0x200000, 0x80000, 8) & 4;
        if (!r)
            goto fail;
        if (entity->timer++ <= 0xB)
            break;
        entity->subState = 2;
        entity->velX.whole = zero;
        entity->animFrame = zero;
        entity->timer = zero;
        Entity_UpdateBoxes(entity);
        break;
    case 10:
        if (entity->animStep != 1)
            break;
        entity->subState++;
        entity->accelX.whole = 0;
        entity->velY.whole = 0;
        entity->accelY.whole = 0;
        if (entity->flipX)
            entity->velX.whole = 0x100000;
        else
            entity->velX.whole = -0x100000;
        EntityAccelerateAndMove(entity, 0x100000);
        WEREJAGUAR_CONTACT_KIND(entity) = 3;
        if (entity->flipX)
            entity->velX.whole = 0x10000;
        else
            entity->velX.whole = -0x10000;
        entity->velY.whole = -0x40000;
        entity->accelY.whole = 0x3000;
        PlaySong(0x86);
        Afterimage_Create(entity, 0x38, 0xC, 1);
        Afterimage_Create(entity, 0x38, 6, 0);
        break;
    case 11:
        {
            s32 z = 0;
            s32 v;
            WEREJAGUAR_CONTACT_KIND(entity) = 3;
            if (entity->animStep == 3)
            {
                entity->animFlags |= 2;
                WEREJAGUAR_CONTACT_KIND(entity) = z;
            }
            v = 0x20000;
            d = Entity_MoveWithCollision(entity, v, -0x200000, 0x80000, 8);
            if (!(d & 4))
                break;
            entity->animFlags &= ~2;
            entity->animTimer = 0xFA;
            entity->velX.whole = z;
            entity->accelX.whole = z;
            entity->velY.whole = v;
            entity->accelY.whole = 0x2800;
            entity->subState++;
            entity->timer = z;
            break;
        }
    case 12:
        if (entity->timer++ <= 0x1D)
            break;
        Anim_StartIndexed(entity, 6, 0, 1);
        entity->state = 0;
        entity->subState = 1;
        break;
    case 13:
        zero = 0;
        WEREJAGUAR_MANUAL_ANIM(entity) = 1;
        r = Entity_MoveWithCollision(entity, 0x40000, -0x200000, 0x80000, 8) & 4;
        if (!r)
            goto fail;
        if (entity->timer++ <= 3)
            break;
        entity->subState = 5;
        entity->timer = zero;
        Anim_StartIndexed(entity, 0, 0, 0);
        WEREJAGUAR_MANUAL_ANIM(entity) = zero;
        break;
    fail:
        entity->state = r;
        entity->subState = r;
        Anim_StartIndexed(entity, 0, 0, 1);
        WEREJAGUAR_MANUAL_ANIM(entity) = r;
        break;
    }
}
