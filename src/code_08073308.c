#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern void ManticoreTailSegment_Update(struct EwramData_EntityData *);
extern u8 sManticoreGfx[], sManticoreSpriteSheet[];
struct EwramData_EntityData *Manticore_CreateTailSegment(struct EwramData_EntityData *owner, struct EwramData_EntityData *part, s32 value, s32 kind)
{
    struct EwramData_EntityData *entity = Entity_CreateObject(ManticoreTailSegment_Update);
    if (entity != NULL)
    {
        s32 graphics;
        u16 slot;
        entity->spriteLayer = part->spriteLayer + 1;
        graphics = ObjGfx_Find(sManticoreGfx);
        Sprite_Attach(entity, sManticoreSpriteSheet, graphics, owner->unk_510.part8.basePalette);
        if (kind == 5) entity->animFrame = 1;
        else entity->animFrame = 2;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot >= 0)
        {
            entity->affineIndex = slot;
            entity->affine = 1;
            entity->unk_518.part8.enemyId = part->unk_518.part8.enemyId;
            entity->scrollWithBg = 0;
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            entity->unk_4F8.unk_4F8_entity = owner;
            entity->unk_4FC.unk_4FC_entity = part;
            entity->unk_500.part8.unk_500 = kind;
            entity->unk_504.whole = value;
            return entity;
        }
        EntityDelete(entity);
    }
    return NULL;
}
