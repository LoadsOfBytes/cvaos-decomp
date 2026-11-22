/* Object13ArmView segment object: copies the angle of entity[unk_511], places itself
 * at half the (cos, sin) * 8 offset from its parent (unk_4FC) and keeps the
 * full offset for the next segment; rotated via ObjAffine_Set. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, void *resource, void *palette, u8 variant, void *sheet);
extern u8 sObject13FamilyGfx[], sUnk_08209FD8[], sObject13FamilySpriteSheet[];

struct Object13ArmView
{
    void *parent;
    s32 angle;
    s32 tipTargetAngle;
    s32 x;
    s32 y;
    u8 segmentIndex;
    u8 ownerSlot;
};
#define ARM(e) ((struct Object13ArmView *)&((struct EwramData_EntityData *)(e))->unk_4FC)

/**
 * @brief One of the ten affine segments of object 0x13, linked to the previous
 * part.
 */
void Object13Segment_Update(struct EwramData_EntityData *entity)
{
    struct Object13ArmView *self = ARM(entity);
    struct Object13ArmView *p = self->parent;
    struct EwramData_EntityData *other = &gEwramData->entityData[self->ownerSlot];
    struct Object13ArmView *a = ARM(other);

    if (other->state == 0xFF)
        entity->state = 0xFF;
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sObject13FamilyGfx, sUnk_08209FD8, 0, sObject13FamilySpriteSheet))
            goto delete;
        Anim_StartIndexed(entity, 1, 0, 0);
        Anim_Update(entity);
        entity->spriteLayer = 3;
        entity->state++;
    case 1:
        self->angle = a->angle;
        {
        struct Object13ArmView *a2 = ARM(entity), *p2 = ARM(p);
        a2->y = Sin(a2->angle + 0x4000) << 3;
        a2->x = Sin(a2->angle + 0x8000) << 3;
        entity->posX.whole = (a2->x >> 1) + p2->x;
        entity->posY.whole = a2->y / 2 + p2->y;
        a2->x += p2->x;
        a2->y += p2->y;
        }
        break;
    case 0xFF:
        ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    ObjAffine_Set(self->angle, 0x10000, 0x10000, 0, 0, entity->affineIndex);
}
