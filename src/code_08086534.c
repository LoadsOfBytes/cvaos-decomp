/* Gargoyle: AI, hit reaction and graphics. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern void EntityMove(struct EwramData_EntityData *);
extern void EntityMoveY(struct EwramData_EntityData *);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void EntityDampVelocityY(struct EwramData_EntityData *, s32);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *, s32);
extern void EntityAccelerateAndMoveX(struct EwramData_EntityData *, s32);
extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern u8 sGargoyleGfx[];
extern u8 sGargoylePalette[];
extern u8 sGargoyleSpriteSheet[];
#define ACCEL(e) ((e)->accelX.whole)
#define MAX_SPEED(e) ((e)->accelY.whole)
/* Gargoyle AI: fly towards a point 64 px above the target, swoop down,
 * glide along and climb back. */
void Gargoyle_UpdateFly(struct EwramData_EntityData *entity)
{
    s32 x, y;
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        entity->velX.whole = entity->flipX ? 0x8000 : -0x8000;
        entity->velY.whole = 0;
        ACCEL(entity) = 0x1000;
        MAX_SPEED(entity) = 0x10000;
        entity->timer = 0;
        entity->subState++;
    case 1:
        x = Target_GetX(entity) - entity->posX.whole;
        {
            s32 target = Target_GetY(entity);
            s32 base = entity->posY.whole + 0x400000;
            y = target - base;
        }
        if (x < -0x80000)
        {
            entity->velX.whole -= ACCEL(entity);
            if (entity->velX.whole <= -MAX_SPEED(entity))
                entity->velX.whole = -MAX_SPEED(entity);
        }
        else if (x > 0x80000)
        {
            entity->velX.whole += ACCEL(entity);
            if (entity->velX.whole >= MAX_SPEED(entity))
                entity->velX.whole = MAX_SPEED(entity);
        }
        if (y < -0x80000)
        {
            entity->velY.whole -= ACCEL(entity);
            if (entity->velY.whole <= -MAX_SPEED(entity))
                entity->velY.whole = -MAX_SPEED(entity);
        }
        else if (y > 0x80000)
        {
            entity->velY.whole += ACCEL(entity);
            if (entity->velY.whole >= MAX_SPEED(entity))
                entity->velY.whole = MAX_SPEED(entity);
        }
        if (entity->velX.whole >= 0) entity->flipX = 1;
        else entity->flipX = 0;
        EntityMove(entity);
        if ((s8)--entity->timer <= 0)
        {
            entity->timer = 0;
            if (y > -0x80000 && y < 0x80000 && (x < 0 ? -x : x) < 0x600000)
            {
                entity->subState++;
                Anim_StartIndexed(entity, 3, 0, 0);
            }
        }
        break;
    case 2:
        entity->flipX = Entity_IsTargetRight(entity);
        EntityDampVelocityX(entity, 10);
        EntityDampVelocityY(entity, 10);
        EntityMove(entity);
        if (entity->timer++ > 19)
        {
            entity->subState++;
            entity->timer = 0;
            Anim_StartIndexed(entity, 1, 0, 0);
            ACCEL(entity) = entity->flipX ? 0xC00 : -0xC00;
            MAX_SPEED(entity) = 0x1800;
            PlaySong(0x87);
        }
        break;
    case 3:
        if (RandomNumberGenerator() % 256 == 0) Anim_StartIndexed(entity, 2, 0, 0);
        EntityAccelerateAndMove(entity, 0x20000);
        if (entity->timer++ > 23)
        {
            entity->subState++;
            entity->timer = 0;
            ACCEL(entity) *= 2;
        }
        break;
    case 4:
        if (RandomNumberGenerator() % 256 == 0) Anim_StartIndexed(entity, 2, 0, 0);
        EntityAccelerateAndMoveX(entity, 0x20000);
        EntityDampVelocityY(entity, 0x10);
        EntityMoveY(entity);
        if (entity->timer++ > 59)
        {
            entity->subState++;
            entity->timer = 0;
            ACCEL(entity) = 0x1000;
            MAX_SPEED(entity) = 0x18000;
            Anim_StartIndexed(entity, 0, 0, 1);
        }
        break;
    case 5:
        x = Target_GetX(entity) - entity->posX.whole;
        {
            s32 target = Target_GetY(entity);
            s32 base = entity->posY.whole + 0x400000;
            y = target - base;
        }
        if (x < -0x80000)
        {
            entity->velX.whole -= ACCEL(entity);
            if (entity->velX.whole <= -MAX_SPEED(entity))
                entity->velX.whole = -MAX_SPEED(entity);
        }
        else if (x > 0x80000)
        {
            entity->velX.whole += ACCEL(entity);
            if (entity->velX.whole >= MAX_SPEED(entity))
                entity->velX.whole = MAX_SPEED(entity);
        }
        if (y < 0)
        {
            entity->velY.whole -= ACCEL(entity);
            if (entity->velY.whole <= -MAX_SPEED(entity))
                entity->velY.whole = -MAX_SPEED(entity);
        }
        else
        {
            entity->subState = 1;
            entity->timer = 20;
            ACCEL(entity) = 0x1000;
            MAX_SPEED(entity) = 0x10000;
        }
        if (entity->velX.whole >= 0) entity->flipX = 1;
        else entity->flipX = 0;
        EntityMove(entity);
        break;
    }
}

/* Gargoyle hit: flash, subtract HP; at 0 HP start falling (state 1). */
void Gargoyle_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    entity->unk_510.part8.hitFlashTimer = 8;
    {
        u16 hp = entity->unk_518.part16.hp;
        damage <<= 16;
        damage >>= 16;
        entity->unk_518.part16.hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x6E);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 0x28;
        entity->state = 1;
        entity->subState = 0;
        entity->accelX.whole = 0;
        entity->accelY.whole = 0x1000;
    }
}
/* Load the Gargoyle graphics; palette 0 for entity type 0x57, else 1. */
u8 Gargoyle_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 graphics = ObjGfx_LoadResource(sGargoyleGfx, 0);

    if (graphics < 0)
        return 0;
    if (entity->unk_518.part8.enemyId == 0x57)
        entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sGargoylePalette, 0);
    else
        entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sGargoylePalette, 1);
    Sprite_Attach(entity, sGargoyleSpriteSheet, graphics, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
