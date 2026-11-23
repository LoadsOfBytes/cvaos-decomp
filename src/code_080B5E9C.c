#include "code_08009A0.h"
#include "balore_entity.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern u8 sBaloreSpriteSheet2[];

void BaloreDebris_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.balorePart.body;
    if (parent->state == 10) entity->state = 2;
    switch (entity->state)
    {
    case 0:
        Sprite_Attach(entity, sBaloreSpriteSheet2, (s8)parent->ext.balore.gfx,
                     parent->unk_510.part8.basePalette);
        Anim_StartIndexed(entity, entity->subState, 2, 0);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->state = 1;
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        if (entity->animFlags & 4) goto remove;
        break;
    case 2:
remove:
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
}
