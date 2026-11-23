/* Devil part update: follows its joint of the parent's skeleton, aims the
 * sprite along the bone, maintains the hit box and blinks away on death. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
#include "syscalls.h"

struct DevilBox {
    s8 x;
    s8 y;
    s8 width;
    s8 height;
};

extern void *memcpy(void *dst, const void *src, unsigned long size);
extern void Hit_CheckWithPlayerSharedHistory(void *, struct EwramData_EntityData *, struct EwramData_EntityData *);
extern s32 Entity_GetVisibilityPattern(s32);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);
extern const u8 sDevilPartTimers[8];

#define JOINT(e) ((s8)e->ext.jointPart.index)
#define FIX_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)
#define HAS_SPRITE(e) (JOINT(e) == 0 || JOINT(e) == 2 || JOINT(e) == 3 || JOINT(e) == 4 || JOINT(e) == 6 || JOINT(e) == 7)

/**
 * @brief Part entity update: follow its joint of the parent's skeleton, aim the
 * sprite along the bone, maintain the hit box and blink away on death.
 */
void DevilPart_Update(struct EwramData_EntityData *entity)
{
    u8 hit[16];
    u8 timers[8];
    struct DevilBox box;
    struct EwramData_EntityData *parent = entity->ext.jointPart.owner;
    struct DevilPart *parts;
    s32 size;
    s32 x;
    s32 y;

    memcpy(timers, sDevilPartTimers, sizeof(timers));
    if (entity->state == 0)
    {
        if (parent != NULL && parent->updateFunc != NULL
            && parent->unk_518.part8.enemyId == entity->unk_518.part8.enemyId)
        {
            parts = parent->ext.devil.parts;
            entity->palette = parent->palette;
            if (!parent->hidden && (s8)parent->ext.devilParts.hidden == 0)
                entity->hidden = 0;
            else
                entity->hidden = 1;
            entity->posX.whole = parent->posX.whole + parts[JOINT(entity)].offsetX;
            entity->posY.whole = parent->posY.whole + parts[JOINT(entity)].offsetY;
            if (JOINT(entity) == 2 || JOINT(entity) == 6)
            {
                x = parts[(s8)entity->ext.jointPart.anchor].offsetX;
                x -= parts[JOINT(entity)].offsetX;
                y = parts[(s8)entity->ext.jointPart.anchor].offsetY;
                y -= parts[JOINT(entity)].offsetY;
                entity->velX.whole = ArcTan2(FIX_INT(x), FIX_INT(y)) - 0x4000;
            }
            else if (JOINT(entity) == 0 || JOINT(entity) == 3 || JOINT(entity) == 4 || JOINT(entity) == 7)
            {
                entity->velX.whole = parts[JOINT(entity)].rotationAngle;
            }
            if (parent->state == 2)
            {
                entity->state = 1;
                entity->subState = 0;
                entity->ext.devilPart.angle = entity->velX.whole;
                entity->timer = timers[JOINT(entity)];
                return;
            }
            if ((parent->ext.joint.hurtMask >> JOINT(entity)) & 1)
            {
                size = 8;
                x = Sin(parts[JOINT(entity)].rotationAngle + 0x8000);
                y = Sin(parts[JOINT(entity)].rotationAngle + 0x4000);
                if (entity->flipX)
                    box.x = -(FIX_INT(x * size) + 4);
                else
                    box.x = FIX_INT(x * size) - 4;
                box.y = FIX_INT(y * size) - 4;
                box.width = 8;
                box.height = 8;
                HitBox_SetBody(entity, &box);
                if (entity->animFrame == 3)
                {
                    size = 0x14;
                    x = Sin(parts[JOINT(entity)].rotationAngle + 0x8000);
                    y = Sin(parts[JOINT(entity)].rotationAngle + 0x4000);
                    if (entity->flipX)
                        box.x = -(FIX_INT(x * size) + 4);
                    else
                        box.x = FIX_INT(x * size) - 4;
                    box.y = FIX_INT(y * size) - 4;
                    box.width = 12;
                    box.height = 12;
                }
                HitBox_SetAttack(entity, &box);
                entity->hit.flags &= 0xFD;
                entity->hit.flags &= 0xFB;
                if ((s8)parent->ext.devil.intangible != 1)
                    Hit_CheckWithPlayerSharedHistory(hit, entity, parent);
            }
            else
            {
                entity->hit.flags |= 4;
                entity->hit.flags |= 2;
            }
            if (HAS_SPRITE(entity))
                ObjAffine_Set(entity->velX.whole, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
            return;
        }
        if (HAS_SPRITE(entity))
            ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
    else
    {
        if (entity->subState == 0)
        {
            entity->subState++;
            entity->scrollWithBg = 1;
        }
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((u8)(entity->timer - 0x14) <= 0x14 && (entity->timer & 3) == 0)
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0x100000, 0x100000, entity->spriteLayer - 1);
        if (HAS_SPRITE(entity))
            ObjAffine_Set(entity->ext.devilPart.angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
        if ((s8)--entity->timer <= 0)
        {
            if (HAS_SPRITE(entity))
                ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
        }
    }
}
