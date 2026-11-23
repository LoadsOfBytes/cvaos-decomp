/* Belmont projectile wave: nineteen delayed entities around the owner. */
#include "code_08009A0.h"
#include "belmont_entity.h"
#include "structs/ewram.h"
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern void BelmontWaveProjectile_Update(struct EwramData_EntityData *);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *,u8 *,void *,u8,void *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *,void *,s32);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern u8 sJuliusPalette[],sBelmontSpriteSheet[],sBelmontWeaponGfx[];
#define ABS(x) ((x)<0?-(x):(x))
/**
 * @brief Create up to nineteen delayed wave projectiles at spaced offsets
 * from the owner, copying its attack damage. Stop on allocation or
 * graphics failure and preserve the asymmetric loop range -10 through 9.
 */
void Belmont_SpawnProjectileWave(struct EwramData_EntityData *entity)
{
    void *palette=sJuliusPalette;
    void *sprite=sBelmontSpriteSheet;
    u8 *graphics=sBelmontWeaponGfx;
    s32 i=-10;
    s32 offset=-0x8C0000;
    do
    {
        struct EwramData_EntityData *projectile;
        if(i!=0)
        {
        projectile=Entity_CreateObject(BelmontWaveProjectile_Update);
        if(!projectile)
            return;
        if(!Entity_LoadGraphics(projectile,graphics,palette,1,sprite))
        {
            EntityDelete(projectile);
            return;
        }
        projectile->posX.whole=entity->posX.whole+offset;
        projectile->posY.whole=entity->posY.whole+0x200000;
        projectile->timer=ABS(i);
        ENTITY_PARENT_INDEX(projectile)=ENTITY_INDEX(entity);
        projectile->spriteLayer=25;
        projectile->flipX=entity->flipX;
        projectile->scrollWithBg=1;
        projectile->animFrame=projectile->unk_510.part8.basePalette;
        projectile->hidden=1;
        Entity_EnableBoxes(projectile,Enemy_OnContact,0);
        projectile->hit.flags|=4;
        BELMONT_ATTACK(projectile)=BELMONT_ATTACK(entity);
        projectile->attack=BELMONT_ATTACK(entity);
        }
        offset+=0xE0000;
        i++;
    } while(i<=9);
}

