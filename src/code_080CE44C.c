/* Spawn a translucent copy of Alastor with the current frame/angle. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void AlastorAfterimage_Update(struct EwramData_EntityData *);
/**
 * @brief Create a translucent sprite copy at the current frame and pose
 * with a timer and owner link. Allocate its own affine slot and return
 * the copy or NULL; preserve existing allocation and cleanup behavior.
 */
struct EwramData_EntityData *Alastor_CreateAfterimage(struct EwramData_EntityData *entity,u8 timer)
{
    s32 graphics=Sprite_GetTileSlot(entity);
    u8 *graphicsIndex=&entity->spriteSlot;
    struct SpriteSheet *source=gEwramData->spriteSlots[*graphicsIndex].sheet;
    struct SpriteSheet *sprite=source;
    struct EwramData_EntityData *copy=EntityCreateInRange(0x49,0xCF,(u32 *)AlastorAfterimage_Update);
    s32 affine;
    if(!copy)
        return NULL;
    if(!source) { EntityDelete(copy); return NULL; }
    affine=ObjAffine_Alloc(1);
    if(affine<0)
    {
        EntityDelete(copy);
        return NULL;
    }
    Sprite_Attach(copy,sprite,graphics,entity->palette);
    copy->animFrame=entity->animFrame;
    copy->spriteLayer=entity->spriteLayer+1;
    copy->posX.whole=entity->posX.whole;
    copy->posY.whole=entity->posY.whole;
    copy->scrollWithBg=1;
    copy->objMode=1;
    copy->flipX=entity->flipX;
    copy->drawFunc=(u32 *)Sprite_DrawEntity;
    copy->affine=entity->affine;
    if(copy->affine)
    {
        copy->affineIndex=affine;
        ObjAffine_Set(entity->ext.alastor.angle,0x10000,0x10000,copy->flipX,0,copy->affineIndex);
    }
    ENTITY_PARENT_INDEX(copy)=ENTITY_INDEX(entity);
    copy->timer=timer;
    return copy;
}
