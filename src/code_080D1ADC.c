/* Belmont attached weapon entity and its independent graphics slot. */
#include "code_08009A0.h"
#include "belmont_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern void BelmontAttachedWeapon_Update(struct EwramData_EntityData *);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *,u8 *,void *,u8,void *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *,void *,s32);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern u8 sJuliusPalette[],sBelmontSpriteSheet[],sBelmontWeaponGfx[];
/**
 * @brief Create the powered-phase attached weapon with its own graphics
 * setup, owner slot links and stored damage 75. Start hidden/inert;
 * return 1 on success or 0 after entity/graphics failure cleanup.
 */
s32 Belmont_CreateAttachedWeapon(struct EwramData_EntityData *entity)
{
    void *palette=sJuliusPalette;
    void *sprite=sBelmontSpriteSheet;
    u8 *graphics=sBelmontWeaponGfx;
    struct EwramData_EntityData *weapon=Entity_CreateObject(BelmontAttachedWeapon_Update);
    if(!weapon) return 0;
    if(!Entity_LoadGraphics(weapon,graphics,palette,1,sprite))
    { EntityDelete(weapon); return 0; }
    {
            Anim_StartIndexed(weapon,0,0,1);
            Anim_Update(weapon);
            ENTITY_PARENT_INDEX(weapon)=ENTITY_INDEX(entity);
            BELMONT_WEAPON_INDEX(entity)=ENTITY_INDEX(weapon);
            weapon->spriteLayer=entity->spriteLayer+1;
            weapon->flipX=entity->flipX;
            weapon->posX.whole=entity->posX.whole;
            weapon->posY.whole=entity->posY.whole;
            weapon->scrollWithBg=1;
            weapon->hidden=1;
            weapon->attack=75;
            weapon->unk_518.part8.enemyId=entity->unk_518.part8.enemyId;
            Entity_EnableBoxes(weapon,Enemy_OnContact,0);
            return 1;
    }
}
