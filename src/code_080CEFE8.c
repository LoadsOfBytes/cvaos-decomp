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
        part->unk_4F4.part8.unk_4F5=entity->unk_4F4.part8.unk_4F4;
        entity->unk_4F4.part8.unk_4F6=part->unk_4F4.part8.unk_4F4;
        part->spriteLayer=entity->spriteLayer-1;
        part->unk_510.part8.basePalette=entity->unk_510.part8.basePalette;
        part->flipX=entity->flipX;
        part->posX.whole=entity->posX.whole;
        part->posY.whole=entity->posY.whole;
        part->scrollWithBg=1;
        part->unk_518.part8.spawnIndex=entity->unk_518.part8.spawnIndex;
        part->unk_518.part8.enemyId=entity->unk_518.part8.enemyId;
        part->unk_518.part16.hp=entity->unk_518.part16.hp;
        part->unk_510.part16.unk_512=entity->unk_510.part16.unk_512;
        part->attack=entity->attack;
        part->defense=entity->defense;
        part->drawFunc=(u32 *)Sprite_DrawEntity;
        sprite=sMandragoraSpriteSheet;
        graphics=Sprite_GetTileSlot(entity);
        Sprite_Attach(part,sprite,graphics,part->unk_510.part8.basePalette);
        Anim_StartIndexed(part,3,0,1);
        Anim_Update(part);
        Entity_EnableBoxes(part,Enemy_OnContact,(s32)MandragoraPart_OnHit);
        if(Mandragora_CreateTerminalPart(part))
            return 1;
        EntityDelete(part);
    }
    return 0;
}
