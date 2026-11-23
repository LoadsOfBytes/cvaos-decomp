/* Minotaur / Red Minotaur helpers 0x080B0CCC..0x080B0D5B. */
#include "code/code_080211F0.h"
#include "structs/ewram.h"

extern void Minotaur_Init(struct EwramData_EntityData *entity, s32 variant);
extern void Minotaur_Update(struct EwramData_EntityData *entity);

/**
 * @brief B0CCC | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position.
 */
void Minotaur_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief B0CFC | Player_ApplyContactDamage with (attack, 1, 0, 1) at the entity position
 * (same as Buer_DamagePlayerOnContact).
 */
void MinotaurAxe_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 1;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief B0D2C | Minotaur create: Minotaur_Init variant 0.
 */
void EnemyMinotaurCreate(struct EwramData_EntityData *entity)
{
    Minotaur_Init(entity, 0);
}

/**
 * @brief B0D38 | Minotaur update.
 */
void EnemyMinotaurUpdate(struct EwramData_EntityData *entity)
{
    Minotaur_Update(entity);
}

/**
 * @brief B0D44 | Red Minotaur create: variant 1.
 */
void EnemyRedMinotaurCreate(struct EwramData_EntityData *entity)
{
    Minotaur_Init(entity, 1);
}

/**
 * @brief B0D50 | Red Minotaur update: same as Minotaur.
 */
void EnemyRedMinotaurUpdate(struct EwramData_EntityData *entity)
{
    Minotaur_Update(entity);
}

/* Axe Armor 0x080B0D5C..0x080B136F; only matches without -f2003-patch. */
#include "code_08009A0.h"
#include "code/code_08039340.h"

extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern void Effect_SpawnRandom(s32 x, s32 y, s32 width, s32 height, s32 layer);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, u8 variant, void *sheet);
extern void PlaySong(u16 song);
extern s32 Hit_Enable(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);
extern s32 HitBox_SetAttackRaw(struct EwramData_EntityData *entity, s32 box);
extern s32 HitBox_SetBodyRaw(struct EwramData_EntityData *entity, s32 box);
extern void Enemy_OnContactMask1(struct EwramData_EntityData *entity);
extern void AxeArmorAxe_ClearHealth(struct EwramData_EntityData *entity);
extern void EntityAccelerateAndMoveX(struct EwramData_EntityData *entity, s32 maxSpeed);
extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *entity, s32 margin);
extern s32 Entity_Walk(struct EwramData_EntityData *entity, s32 speed, void *profile, s32 flags);
extern s32 Entity_GetTargetDirection(struct EwramData_EntityData *entity);
extern s16 Entity_GetTargetDistance(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *updateFunc);
extern s32 RandomSigned(s32 range);
extern struct EwramData_EntityData *Effect_SpawnShared(s32 x, s32 y, s32 script, s32 palette);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_EnableBoxes(struct EwramData_EntityData *entity, void *onHit, s32 onHurt);
extern void Enemy_OnContact(struct EwramData_EntityData *entity);
extern void AxeArmor_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hitData);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);

/* Graphics of the Axe Armor and its axe; walk speeds per animation frame. */
extern u8 sAxeArmorGfx[];
extern u8 sAxeArmorPalette[];
extern u8 sAxeArmorSpriteSheet[];
extern u8 sAxeArmorWalkProfile[];

/**
 * @brief Initialize and update the spinning thrown axe; release its affine slot on destruction.
 */
void AxeArmorAxe_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    u16 slot;

    if (entity->state != 0)
    {
        Hit_CheckWithPlayer(collision, entity);
        if ((s16)entity->hp <= 0)
        {
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0x80000, 0x80000, entity->spriteLayer - 1);
            entity->state = 2;
        }
        else if (gEwramData->gameState.timeStopTimer != 0)
        {
            return;
        }
    }
    switch (entity->state)
    {
        case 0:
            if (!Entity_LoadGraphics(entity, sAxeArmorGfx, sAxeArmorPalette, 0, sAxeArmorSpriteSheet))
                goto delete;
            slot = ObjAffine_Alloc(1);
            if ((s16)slot < 0)
                goto delete;
            entity->affineIndex = slot;
            entity->affine = 1;
            Anim_StartIndexed(entity, 3, 0, 0);
            Anim_Update(entity);
            entity->spriteLayer = 0x14;
            entity->scrollWithBg = 1;
            entity->velX.whole = -0x28000;
            entity->accelX.whole = 0x800;
            entity->ext.axeArmorAxe.spin = 0x800;
            if (entity->stage == 1)
            {
                entity->posY.part16.integer -= 20;
                entity->ext.axeArmorAxe.spin = -entity->ext.axeArmorAxe.spin;
            }
            if (entity->flipX)
            {
                entity->velX.whole = -entity->velX.whole;
                entity->accelX.whole = -entity->accelX.whole;
                entity->ext.axeArmorAxe.spin = -entity->ext.axeArmorAxe.spin;
            }
            entity->state = 1;
            PlaySong(0x86);
            entity->hp = 1;
            Hit_Enable(entity, 10, Enemy_OnContactMask1, (s32)AxeArmorAxe_ClearHealth);
            HitBox_SetAttackRaw(entity, 0x1010F8F8);
            HitBox_SetBodyRaw(entity, 0x1010F8F8);
            break;
        case 1:
            EntityAccelerateAndMoveX(entity, 0x28000);
            break;
        case 2:
            ObjAffine_Free(entity->affineIndex);
        delete:
            EntityDelete(entity);
            return;
    }
    entity->ext.axeArmorAxe.angle += entity->ext.axeArmorAxe.spin;
    ObjAffine_Set(entity->ext.axeArmorAxe.angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
    if (EntityIsOutsideScreen(entity, 4))
        entity->state = 2;
}

/**
 * @brief Walk at animation-dependent speed; turn or enter attack state 1 at the designated animation step.
 */
void AxeArmor_UpdateWalking(struct EwramData_EntityData *entity)
{
    u8 frame;
    s32 speed;
    s32 result;
    s32 attack;
    s32 distance;

    if (entity->subState != 0)
        return;
    // animStep and animTimer read together: frame 0x11 and animTimer 0
    if (*(u16 *)&entity->animStep == 0x11 && entity->spawnParam0 != 0
        && entity->timer++ > entity->spawnParam0)
    {
        entity->timer = entity->subState;
        entity->flipX ^= 1;
    }
    frame = entity->animStep % 9;
    speed = frame * 0x2800 + 0x1000;
    if (frame > 6)
        speed = 0;
    result = Entity_Walk(entity, speed, sAxeArmorWalkProfile, 20);
    if (entity->spawnParam0 == 0 && (result & 0x1B) != 8)
        entity->flipX ^= 1;
    if (entity->animTimer == 0)
    {
        if (entity->animStep != 0x11)
            return;
        // attack = target in front && distance <= 99, spelled out for the
        // original flag order
        if ((entity->flipX && (s8)Entity_GetTargetDirection(entity) == 1)
            || (!entity->flipX && (s8)Entity_GetTargetDirection(entity) == -1))
        {
            distance = Entity_GetTargetDistance(entity);
            attack = 1;
            if (distance <= 99)
                goto test;
        }
        attack = 0;
    test:
        if (attack)
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
    }
}
