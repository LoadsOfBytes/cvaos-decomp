/* Stolas part update: follows its joint of the parent's skeleton, aims the
 * sprite along the bone and maintains the hit boxes of joints 0 and 2. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
#include "syscalls.h"

struct StolasBox {
    s8 x;
    s8 y;
    s8 width;
    s8 height;
};

extern void *memcpy(void *dst, const void *src, unsigned long size);
extern void Hit_CheckWithPlayerSharedHistory(void *, struct EwramData_EntityData *, struct EwramData_EntityData *);
extern const u8 sUnk_081190B2[13];

#define JOINT(e) ((s8)e->ext.jointPart.index)
#define FIX_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)

void StolasPart_Update(struct EwramData_EntityData *entity)
{
    u8 hit[16];
    u8 unused[13];
    struct StolasBox box;
    struct EwramData_EntityData *parent = entity->ext.jointPart.owner;
    struct EntityJointPart *parts;
    s32 size = 8;
    s32 x;
    s32 y;
    u16 angle;
    s32 state;

    memcpy(unused, sUnk_081190B2, sizeof(unused));
    parts = parent->ext.joint.parts;
    state = entity->state;
    if (state == 0)
    {
        if (parent != NULL && parent->updateFunc != NULL
            && parent->unk_518.part8.enemyId == entity->unk_518.part8.enemyId)
        {
            entity->palette = parent->palette;
            entity->flipX = parent->flipX;
            entity->timer++;
            entity->posX.whole = parent->posX.whole + parts[JOINT(entity)].offsetX;
            entity->posY.whole = parent->posY.whole + parts[JOINT(entity)].offsetY;
            if (entity->ext.jointPart.index <= 1 || JOINT(entity) == 3 || JOINT(entity) == 4)
            {
                x = parts[(s8)entity->ext.jointPart.anchor].offsetX;
                x -= parts[JOINT(entity)].offsetX;
                y = parts[(s8)entity->ext.jointPart.anchor].offsetY - parts[JOINT(entity)].offsetY;

                angle = ArcTan2(FIX_INT(x), FIX_INT(y));

                if (entity->flipX)
                    entity->velX.whole = angle;
                else
                    entity->velX.whole = angle - 0x8000;
                parts[JOINT(entity)].rotationAngle = entity->velX.whole;
            }
            else if (JOINT(entity) == 2 || JOINT(entity) == 5)
            {
                entity->velX.whole = parts[JOINT(entity)].rotationAngle;
            }
            if (parent->state == 2)
            {
                entity->state = 1;
                entity->subState = 0;
                entity->timer = 0x3C;
                return;
            }
            if ((parent->ext.joint.hurtMask >> JOINT(entity)) & 1)
            {
                if (JOINT(entity) == 0)
                {
                    box.x = -16;
                    box.y = -24;
                    box.width = 32;
                    box.height = 24;
                    HitBox_SetAttack(entity, &box);
                    HitBox_SetBody(entity, &box);
                    entity->hit.flags &= 0xFB;
                    entity->hit.flags &= 0xFD;
                    Hit_CheckWithPlayerSharedHistory(hit, entity, parent);
                }
                else if (JOINT(entity) == 2)
                {

                    if (entity->flipX)
                    {
                        x = Sin(-(parts[JOINT(entity)].rotationAngle + 0x8000) + 0x4000);
                        y = Sin(-(parts[JOINT(entity)].rotationAngle + 0x8000));
                    }
                    else
                    {
                        x = Sin(parts[JOINT(entity)].rotationAngle + 0xC000);
                        y = Sin(parts[JOINT(entity)].rotationAngle + 0x8000);
                    }
                    { s32 px = x * size; box.x = FIX_INT(px) - 3; }
                    { s32 py = y * size; box.y = FIX_INT(py) - 3; }
                    box.width = 8;
                    box.height = 8;
                    HitBox_SetAttack(entity, &box);
                    entity->hit.flags &= 0xFD;
                    entity->hit.flags &= 0xFB;
                    Hit_CheckWithPlayerSharedHistory(hit, entity, parent);
                }
            }
            else
            {
                entity->hit.flags |= 4;
                entity->hit.flags |= 2;
            }
            ObjAffine_Set(entity->velX.whole, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
            return;
        }
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
    else if (state == 1)
    {
        entity->posX.whole = parent->posX.whole + parts[JOINT(entity)].offsetX;
        entity->posY.whole = parent->posY.whole + parts[JOINT(entity)].offsetY;
        state = parent->hidden;
        entity->hidden = state;
        if ((s8)--entity->timer <= 0)
        {
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
        }
    }
}
