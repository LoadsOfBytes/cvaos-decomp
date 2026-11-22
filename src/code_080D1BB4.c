/* Belmont subweapon factory. All callers supply variants 0 through 3. */
#include "code_08009A0.h"
#include "structs/ewram.h"
extern void BelmontSubweapon0_Update(struct EwramData_EntityData *);
extern void BelmontSubweapon1_Update(struct EwramData_EntityData *);
extern void BelmontSubweapon2_Update(struct EwramData_EntityData *);
extern void BelmontSubweapon3_Update(struct EwramData_EntityData *);
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *,u8 *,void *,u8,void *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *,void *,s32);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern u8 sJuliusPalette[],sBelmontSpriteSheet[],sBelmontWeaponGfx[];
/**
 * @brief Create subweapon variant 0 through 3 with its selected update
 * callback, graphics palette, owner link and spawn offset. Return the
 * entity or NULL. Preserve the existing valid-variant precondition.
 */
struct EwramData_EntityData *Belmont_CreateSubweapon(struct EwramData_EntityData *entity,s32 kind)
{
    void *palette=sJuliusPalette;
    void *sprite=sBelmontSpriteSheet;
    u8 *graphics=sBelmontWeaponGfx;
    s32 slot=1;
    void (*update)(struct EwramData_EntityData *);
    struct EwramData_EntityData *weapon;
    switch(kind)
    {
        case 0: update=BelmontSubweapon0_Update; slot=0; break;
        case 1: update=BelmontSubweapon1_Update; break;
        case 2: update=BelmontSubweapon2_Update; break;
        case 3: update=BelmontSubweapon3_Update; break;
    }
    weapon=Entity_CreateObject(update);
    if(!weapon)
        return NULL;
    if(!Entity_LoadGraphics(weapon,graphics,palette,slot,sprite))
    {
        EntityDelete(weapon);
        return NULL;
    }
    weapon->unk_510.part8.basePalette=kind;
    weapon->unk_4F4.part8.unk_4F5=entity->unk_4F4.part8.unk_4F4;
    weapon->spriteLayer=entity->spriteLayer-1;
    weapon->flipX=entity->flipX;
    weapon->posX.whole=entity->posX.whole;
    weapon->posY.whole=entity->posY.whole;
    weapon->scrollWithBg=1;
    weapon->animFrame=weapon->unk_510.part8.basePalette;
    weapon->hidden=1;
    if(kind==0 || kind==2)
    {
        s32 offset=-16;
        if(weapon->flipX)
            offset=16;
        weapon->posX.part16.integer+=offset;
        weapon->posY.part16.integer-=26;
    }
    else if(kind==1)
    {
        s32 offset=-19;
        if(weapon->flipX)
            offset=19;
        weapon->posX.part16.integer+=offset;
        weapon->posY.part16.integer-=6;
    }
    Entity_EnableBoxes(weapon,Enemy_OnContact,0);
    weapon->hit.flags|=4;
    weapon->attack=85;
    return weapon;
}
