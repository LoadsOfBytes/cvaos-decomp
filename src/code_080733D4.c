#include "code_08009A0.h"
#include "code/code_08039340.h"
extern u16 ArcTan2(s32, s32);
#include "structs/ewram.h"
extern void ManticoreTail_UpdateSwing(struct EwramData_EntityData *);
#define KIND(e) ((u8)(e)->unk_500.part8.unk_500)
#define DONE(e) (*(u8 *)&(e)->unk_500.part8.unk_501)
#define NEXT(e) ((e)->unk_504.unk_504_entity)
static inline s32 ToPixels(s32 x) { if (x < 0) return (s16)-(-x >> 16); return x >> 16; }

/* Update a linked part, once per chain traversal, and face the next part. */
void ManticoreTailSegment_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->unk_4F8.unk_4F8_entity;
    struct EwramData_EntityData *next;
    s32 zero, i, x, y, angle;
    if (owner == NULL || owner->updateFunc == NULL || owner->unk_518.part8.enemyId != 0x36)
        goto remove;
    if (KIND(entity) == 0)
    {
        next = entity;
        zero = 0;
        for (i = 5; i >= 0; i--)
        {
            DONE(next) = zero;
            next = NEXT(next);
        }
    }
    if (DONE(entity)) return;
    next = NEXT(entity);
    if (KIND(entity) != 5 && next->updateFunc != NULL)
        ((void (*)(struct EwramData_EntityData *))next->updateFunc)(next);
    DONE(entity) = 1;
    entity->posX.whole = owner->posX.whole;
    entity->posY.whole = owner->posY.whole;
    if (owner->flipX)
    {
        if (KIND(entity) == 0)
            entity->velX.whole = -((KIND(entity) << 19) + 0x60000);
        else
            entity->velX.whole = -((KIND(entity) << 19) + 0xA0000);
        entity->posX.whole -= 0xC0000;
    }
    else
    {
        if (KIND(entity) == 0)
            entity->velX.whole = (KIND(entity) << 19) + 0x60000;
        else
            entity->velX.whole = (KIND(entity) << 19) + 0xA0000;
        entity->posX.whole += 0xC0000;
    }
    entity->posY.whole -= 0x150000;
    entity->flipX = owner->flipX;
    entity->hidden = owner->hidden;
    ManticoreTail_UpdateSwing(entity);
    entity->palette = owner->palette;
    x = next->posX.whole - entity->posX.whole;
    y = next->posY.whole - entity->posY.whole;
    x = ToPixels(x);
    y = ToPixels(y);
    angle = (u16)ArcTan2(x, y);
    if (!entity->flipX) angle += 0x8000;
    ObjAffine_Set(angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    return;
remove:
    ObjAffine_Free(entity->affineIndex);
    EntityDelete(entity);
}
