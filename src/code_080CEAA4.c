/* Alastor linked part, sharing the owner's graphics. */
#include "code_08009A0.h"
#include "alastor_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s8 ObjPalette_FindOrLoad(void *);
extern void AlastorPart_Update(struct EwramData_EntityData *);
extern u8 sAlastorPalette[],sAlastorSpriteSheet[],sAlastorGfx[];
/**
 * @brief Create a linked part using the owner graphics, store both slot
 * links and initialize its update. Return 1 on success, otherwise 0;
 * preserve the existing sprite-attachment failure cleanup behavior.
 */
s32 Alastor_CreateLinkedPart(struct EwramData_EntityData *entity)
{
    void *palette=sAlastorPalette;
    void *sprite=sAlastorSpriteSheet;
    u8 *graphics=sAlastorGfx;
    void (*update)(struct EwramData_EntityData *)=AlastorPart_Update;
    struct EwramData_EntityData *part=EntityCreateInRange(0x49,0xCF,(u32 *)update);
    if (part)
    {
        s32 graphic=ObjGfx_Find(graphics);
        s32 slot=ObjPalette_FindOrLoad(palette);
        if (Sprite_Attach(part,sprite,graphic,slot))
        {
            s32 index=ENTITY_INDEX(entity);
            s32 zero=0;
            part->unk_4F4.part8.unk_4F5=index;
            ALASTOR_PART_INDEX(entity)=ENTITY_INDEX(part);
            part->objMode=1;
            part->drawFunc=(u32 *)Sprite_DrawEntity;
            part->scrollWithBg=1;
            part->spriteLayer=entity->spriteLayer+2;
            part->state=1;
            part->stage=zero;
            part->subState=zero;
            update(part);
            return 1;
        }
    }
    return 0;
}

