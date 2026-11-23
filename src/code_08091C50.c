/* Create a falling body part relative to its owner. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
void DevilProjectile_Update(struct EwramData_EntityData *entity);
void DevilProjectile_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern u8 sDevilGfx[], sDevilSpriteSheet[];

/**
 * @brief Spawn a projectile at offset (x, y) from owner with launch velocity
 * (vx, vy) in the owner palette and type; it rides on the owner for timer
 * frames (DevilProjectile_Update) and damages on contact through
 * DevilProjectile_DamagePlayerOnContact.
 */
void DevilProjectile_Spawn(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy, u8 timer)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, (u32 *)DevilProjectile_Update);
    struct HitBoxRecord box = {-5, -5, 10, 10};
    if (part)
    {
        s32 graphics;
        part->posX.whole = owner->posX.whole + x;
        part->posY.whole = owner->posY.whole + y;
        part->velX.whole = vx;
        part->velY.whole = vy;
        part->ext.devilProjectile.offsetX = x;
        part->ext.devilProjectile.offsetY = y;
        part->spriteLayer = owner->spriteLayer - 2;
        graphics = ObjGfx_Find(sDevilGfx);
        part->unk_510.part8.basePalette = owner->unk_510.part8.basePalette;
        Sprite_Attach(part, sDevilSpriteSheet, graphics, part->unk_510.part8.basePalette);
        Anim_StartIndexed(part, 1, 0, 1);
        Anim_Update(part);
        Hit_Enable(part, 8, DevilProjectile_DamagePlayerOnContact, 0);
        HitBox_SetAttack(part, &box);
        part->hit.flags |= 4;
        part->hit.flags &= ~2;
        part->timer = timer;
        part->ext.devilProjectile.release = 0;
        part->unk_518.part8.enemyId = owner->unk_518.part8.enemyId;
        part->attack = owner->attack;
        part->ext.devilProjectile.owner = owner;
        part->scrollWithBg = 0;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
