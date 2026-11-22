#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Object13Segment_Update(struct EwramData_EntityData *);
extern void Object13Tip_Update(struct EwramData_EntityData *);
extern u8 sObject13FamilyGfx[], sUnk_08209FD8[], sObject13FamilySpriteSheet[];
void Object13Create(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *previous;
    s32 i;
    if (!Entity_LoadGraphics(entity, sObject13FamilyGfx, sUnk_08209FD8, 0, sObject13FamilySpriteSheet)) { EntityDelete(entity); return; }
    entity->drawFunc = NULL;
    entity->hidden = 1;
    entity->scrollWithBg = 1;
    previous = entity;
    for (i = 0; i <= 10; i++)
    {
        u8 *data;
        struct EwramData_EntityData *child;
        if (i != 10) child = EntityCreateInRange(0x49, 0xCF, (u32 *)Object13Segment_Update);
        else child = EntityCreateInRange(0x49, 0xCF, (u32 *)Object13Tip_Update);
        if (child == NULL) { entity->state = 0xFF; break; }
        if (i != 10)
        {
            u16 slot = ObjAffine_Alloc(1);
            if ((s16)slot < 0) { entity->state = 0xFF; break; }
            child->affineIndex = slot;
            child->affine = 1;
        }
        child->scrollWithBg = 1;
        /* General-purpose slots: previous part, zeroed value at +4,
         * part index at +0x14 and the owner's value at +0x15. */
        data = (u8 *)&child->unk_4FC;
        child->unk_4FC.unk_4FC_entity = previous;
        *(s32 *)(data + 4) = 0;
        data[0x14] = i;
        data[0x15] = ENTITY_INDEX(entity);
        previous = child;
    }

}
