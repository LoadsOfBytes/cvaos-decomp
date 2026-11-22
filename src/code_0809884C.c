#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern void Effect_SpawnParticleWithDamping(s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern void CreakingSkullProjectile_Update(struct EwramData_EntityData *);
extern void CreakingSkullProjectile_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void CreakingSkullProjectile_OnHit(struct EwramData_EntityData *);
extern u8 sCreakingSkullGfx[], sCreakingSkullSpriteSheet[];

/* Emit eight particles and a projectile from the owner's given offset. */
/**
 * @brief Emit eight particles and create the ranged projectile at the
 * supplied owner-relative offset and velocity. Configure contact/hit
 * callbacks; preserve the existing Anim_Update(owner) call.
 */
void CreakingSkull_SpawnProjectile(struct EwramData_EntityData *owner, s32 x, s32 y, s32 vx, s32 vy)
{
    u32 box = 0x0808FCFC;
    s32 i, particleX, particleY, graphics;
    struct EwramData_EntityData *entity;
    for (i = 0; i < 8; i++)
    {
        particleY = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
        if (owner->flipX) particleX = -(RandomNumberGenerator() & 0x7FFF) - i * 0x3000;
        else particleX = (RandomNumberGenerator() & 0x7FFF) + i * 0x3000;
        Effect_SpawnParticleWithDamping(owner->posX.whole + x, owner->posY.whole + y, particleX, particleY, owner->spriteLayer - 1, 0, 20, 15, 6);
    }
    entity = EntityCreateInRange(0x1B, 0x48, (u32 *)CreakingSkullProjectile_Update);
    if (entity != NULL)
    {
        graphics = ObjGfx_Find(sCreakingSkullGfx);
        if (graphics < 0)
        {
            EntityDelete(entity);
            return;
        }
        Sprite_Attach(entity, sCreakingSkullSpriteSheet, graphics, owner->unk_510.part8.basePalette);
        Anim_StartIndexed(entity, 0, 0, 0);
        Anim_Update(owner);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_Enable(entity, 10, CreakingSkullProjectile_DamagePlayerOnContact, (s32)CreakingSkullProjectile_OnHit);
        HitBox_SetAttack(entity, &box);
        HitBox_SetBody(entity, &box);
        entity->hit.flags &= 0xFB;
        entity->hit.flags &= 0xFD;
        entity->scrollWithBg = 1;
        entity->spriteLayer = owner->spriteLayer - 1;
        ENTITY_OWNER(entity) = owner;
        entity->posX.whole = owner->posX.whole + x;
        entity->posY.whole = owner->posY.whole + y;
        entity->velX.whole = vx;
        entity->velY.whole = vy;
        entity->flipX = owner->flipX;
        entity->attack = owner->attack;
    }
}


