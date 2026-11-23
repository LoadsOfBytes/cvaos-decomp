/* Spawn the projectile ZombieSoldierGrenade_Update at the owner + (x, y) with the given
 * velocity, copying the owner's facing, palette and layer. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern void ZombieSoldierGrenade_Update(struct EwramData_EntityData *);
extern void ZombieSoldierGrenade_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void ZombieSoldierGrenade_OnHit(struct EwramData_EntityData *, s32, u16 *);
extern u8 sZombieSoldierGfx[];
extern u8 sZombieSoldierSpriteSheet[];

/**
 * @brief Throw a grenade from owner + (x, y) with (velX, velY) under gravity;
 * contact ZombieSoldierGrenade_DamagePlayerOnContact, hit
 * ZombieSoldierGrenade_OnHit.
 */
void ZombieSoldier_ThrowGrenade(struct EwramData_EntityData *owner, s32 x, s32 y, s32 velX, s32 velY)
{
    struct EwramData_EntityData *shot = EntityCreateInRange(0x49, 0xCF, (u32 *)ZombieSoldierGrenade_Update);
    struct HitBoxRecord box = {-2, -2, 4, 4};
    if (shot != NULL)
    {
        s32 graphics;
        shot->posX.whole = owner->posX.whole + x;
        shot->posY.whole = owner->posY.whole + y;
        shot->velX.whole = velX;
        shot->velY.whole = velY;
        shot->accelY.whole = 0x1800;
        shot->flipX = owner->flipX;
        shot->spriteLayer = owner->spriteLayer - 1;
        graphics = ObjGfx_Find(sZombieSoldierGfx);
        shot->basePalette = owner->basePalette;
        Sprite_Attach(shot, sZombieSoldierSpriteSheet, graphics, shot->basePalette);
        shot->animFrame = 0x12;
        Hit_Enable(shot, 8, ZombieSoldierGrenade_DamagePlayerOnContact, (s32)ZombieSoldierGrenade_OnHit);
        HitBox_SetAttack(shot, &box);
        HitBox_SetBody(shot, &box);
        shot->hit.flags &= ~4;
        shot->hit.flags &= ~2;
        shot->enemyId = owner->enemyId;
        shot->attack = owner->attack;
        shot->scrollWithBg = 1;
        shot->drawFunc = (u32 *)Sprite_DrawEntity;
        shot->timer = 0;
    }
}
