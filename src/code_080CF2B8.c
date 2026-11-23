/* Mandragora linked part mirrors combat properties and shares graphics. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern void MandragoraTerminalPart_Update(struct EwramData_EntityData *);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void MandragoraPart_OnHit(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *,void *,s32);
extern s32 Mandragora_CreateTerminalPart(struct EwramData_EntityData *);
extern u8 sMandragoraSpriteSheet[];
extern void Hit_Enable(struct EwramData_EntityData *,s32,void *,s32);
extern void HitBox_SetAttackRaw(struct EwramData_EntityData *,s32);
extern s32 sMandragoraAttackBox[];
/**
 * @brief Create the terminal part with shared graphics and copied combat
 * properties, store both slot links and configure its attack box.
 * Return 1 on success or 0 on entity allocation failure.
 */
s32 Mandragora_CreateTerminalPart(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part=Entity_CreateObject(MandragoraTerminalPart_Update);
    if(!part)
        return 0;
    {
        void *sprite;
        s32 graphics;
        ENTITY_PARENT_INDEX(part)=ENTITY_INDEX(entity);
        entity->ext.mandragora.linkIndex=ENTITY_INDEX(part);
        part->spriteLayer=entity->spriteLayer+1;
        part->unk_510.part8.basePalette=entity->unk_510.part8.basePalette;
        part->flipX=entity->flipX;
        part->posX.whole=entity->posX.whole;
        part->posY.whole=entity->posY.whole;
        part->unk_518.part16.hp=entity->unk_518.part16.hp;
        part->unk_510.part16.unk_512=entity->unk_510.part16.unk_512;
        part->attack=entity->attack;
        part->defense=entity->defense;
        part->scrollWithBg=1;
        part->drawFunc=(u32 *)Sprite_DrawEntity;
        part->roomActive=1;
        sprite=sMandragoraSpriteSheet;
        graphics=Sprite_GetTileSlot(entity);
        Sprite_Attach(part,sprite,graphics,part->unk_510.part8.basePalette);
        Anim_StartIndexed(part,4,0,1);
        Anim_Update(part);
        Hit_Enable(part,8,Enemy_OnContact,0);
        HitBox_SetAttackRaw(part,sMandragoraAttackBox[0]);
        part->objMode=1;
        return 1;
    }
}
