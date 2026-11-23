/* Biphron helpers 0x08099ED8..0x0809A197. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

/* animStep and animTimer read as one halfword (as in code_080756FC.c). */
#define FRAME_PAIR(e) (*(u16 *)&(e)->animStep)

extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);
void Biphron_ThrowShot(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern void BiphronFlame_Update(struct EwramData_EntityData *entity);
extern void BiphronShot_Update(struct EwramData_EntityData *entity);
extern void BiphronShot_DamagePlayerOnContact(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

extern u8 sBiphronGfx[];
extern u8 sBiphronSpriteSheet[];
extern u8 sBiphronShotAnimScript[];

/**
 * @brief 99ED8 | Attack: animation 1; at frame 4 spawn the projectile
 * (Biphron_ThrowShot) 0x1A px in front and play sound 0x78; when the
 * animation ends go back to state 0 with animation 0.
 */
void Biphron_UpdateAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->subState++;
            break;
        case 1:
            if (FRAME_PAIR(entity) == 4)
            {
                Biphron_ThrowShot(entity, entity->flipX ? 0x1A0000 : -0x1A0000, 0);
                PlaySong(0x78);
            }
            if (entity->animDone)
            {
                Anim_StartIndexed(entity, 0, 0, 1);
                entity->state = 0;
                entity->subState = 0;
            }
            break;
    }
}

/**
 * @brief 99F50 | Hit: flash (hitFlashTimer = 8), subtract the damage from the HP
 * and show it; at HP <= 0 die (Enemy_DropRewards, animation 3, palette into
 * 0x53E bits 3-6, state 2).
 */
void Biphron_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;

        // (s16)damage, written so that agbcc sign-extends after the HP load
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 0;
        entity->state = 2;
        entity->subState = 0;
    }
}

/**
 * @brief 99FD0 | Spawn the projectile (update BiphronShot_Update, hit callback
 * BiphronShot_DamagePlayerOnContact) at the entity position plus (offsetX, offsetY), thrown
 * forwards (velX +-0x20000, velY 0x40000, accelY 0x2800).
 */
void Biphron_ThrowShot(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)BiphronShot_Update);
    struct SpriteBox box = { -5, -14, 0x14, 0x0E };
    s32 gfx;

    if (child != NULL)
    {
        child->posX.whole = entity->posX.whole + offsetX;
        child->posY.whole = entity->posY.whole + offsetY;
        child->spriteLayer = entity->spriteLayer;
        gfx = ObjGfx_Find(sBiphronGfx);
        child->basePalette = entity->basePalette;
        Sprite_Attach(child, sBiphronSpriteSheet, gfx, child->basePalette);
        child->hidden = 1;
        Hit_Enable(child, 8, BiphronShot_DamagePlayerOnContact, 0);
        HitBox_SetAttack(child, &box);
        child->hit.flags |= 4;
        child->hit.flags &= ~2;
        Entity_SnapToGround(child);
        child->enemyId = entity->enemyId;
        child->attack = entity->attack;
        child->scrollWithBg = 1;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
        child->flipX = entity->flipX;
        if (child->flipX)
            child->velX.whole = 0x20000;
        else
            child->velX.whole = -0x20000;
        child->velY.whole = 0x40000;
        child->accelY.whole = 0x2800;
    }
}

/**
 * @brief 9A0DC | Spawn the projectile's effect (update BiphronFlame_Update) at the
 * entity position plus (offsetX, offsetY), facing like the entity.
 */
void BiphronShot_SpawnFlame(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)BiphronFlame_Update);
    s32 gfx;

    if (child != NULL)
    {
        child->posX.whole = entity->posX.whole + offsetX;
        child->posY.whole = entity->posY.whole + offsetY;
        child->spriteLayer = entity->spriteLayer;
        gfx = ObjGfx_Find(sBiphronGfx);
        child->basePalette = entity->basePalette;
        Sprite_Attach(child, sBiphronSpriteSheet, gfx, child->basePalette);
        Anim_Start(child, (struct AnimScript *)sBiphronShotAnimScript, 0, 0);
        Anim_Update(child);
        child->hit.flags |= 4;
        child->hit.flags |= 2;
        child->enemyId = entity->enemyId;
        child->scrollWithBg = 1;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
        child->flipX = entity->flipX;
    }
}
