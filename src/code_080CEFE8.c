/* Mandragora linked part mirrors combat properties and shares graphics. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern void MandragoraPart_Update(struct EwramData_EntityData *);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void MandragoraPart_OnHit(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *,void *,s32);
extern s32 Mandragora_CreateTerminalPart(struct EwramData_EntityData *);
extern u8 sMandragoraSpriteSheet[];
/**
 * @brief Create the intermediate part with copied combat properties and
 * shared graphics, link both slot indices and create its terminal part.
 * Return 1 on success; delete the intermediate part and return 0 on failure.
 */
s32 Mandragora_CreateLinkedPart(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part=Entity_CreateObject(MandragoraPart_Update);
    if(part)
    {
        void *sprite;
        s32 graphics;
        ENTITY_PARENT_INDEX(part)=ENTITY_INDEX(entity);
        entity->ext.mandragora.linkIndex=ENTITY_INDEX(part);
        part->spriteLayer=entity->spriteLayer-1;
        part->basePalette=entity->basePalette;
        part->flipX=entity->flipX;
        part->posX.whole=entity->posX.whole;
        part->posY.whole=entity->posY.whole;
        part->scrollWithBg=1;
        part->spawnIndex=entity->spawnIndex;
        part->enemyId=entity->enemyId;
        part->hp=entity->hp;
        part->mp=entity->mp;
        part->attack=entity->attack;
        part->defense=entity->defense;
        part->drawFunc=(u32 *)Sprite_DrawEntity;
        sprite=sMandragoraSpriteSheet;
        graphics=Sprite_GetTileSlot(entity);
        Sprite_Attach(part,sprite,graphics,part->basePalette);
        Anim_StartIndexed(part,3,0,1);
        Anim_Update(part);
        Entity_EnableBoxes(part,Enemy_OnContact,(s32)MandragoraPart_OnHit);
        if(Mandragora_CreateTerminalPart(part))
            return 1;
        EntityDelete(part);
    }
    return 0;
}
