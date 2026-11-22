/* Werewolf helpers 0x0806F678..0x0806F817 (EnemyWerewolfCreate like
 * EnemyWerejaguarCreate without Hit_SetTimerOverride). */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];

extern u8 Werewolf_LoadGraphics(struct EwramData_EntityData *entity);
extern void Werewolf_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void Werewolf_OnHit(struct EwramData_EntityData *param_0);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern s32 Hit_EnableContact(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);

void WerewolfShockwave_DamagePlayerOnContact(struct EwramData_EntityData *entity);

/**
 * @brief 6F678 | Effect: states 0 / 1 go to 2; state 2 runs Hit_CheckWithPlayer
 * while animStep is 1..5 and animates until the end (state 3); state 3 sets
 * 0x53D bit 3.
 */
void WerewolfShockwave_Update(struct EwramData_EntityData *entity)
{
    u8 collision[0x10];

    switch (entity->state)
    {
        case 0:
        case 1:
            entity->state = 2;
            break;
        case 2:
            if ((u8)(entity->animStep - 1) <= 4)
                Hit_CheckWithPlayer(collision, entity);
            if (Anim_Update(entity) == 3)
                entity->state = 3;
            break;
        case 3:
            entity->deleteRequested = 1;
            break;
    }
}

/**
 * @brief 6F6D4 | Take the common effect graphics (palette 7, animation 0xA);
 * 0 when they are not loaded.
 */
u8 WerewolfShockwave_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_Find(sCommonObjectGfx);

    if (gfx < 0)
        return 0;
    Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 7);
    Anim_StartIndexed(entity, 0xA, 0, 0);
    Anim_Update(entity);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 6F71C | Box slot 8 with WerewolfShockwave_DamagePlayerOnContact and the box 0x1810E8F8 (x -8,
 * y -24, w 16, h 24 - not a ROM address); clear flags bit 1, set bit 2.
 */
void WerewolfShockwave_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    Hit_Enable(entity, 8, WerewolfShockwave_DamagePlayerOnContact, 0);
    HitBox_SetAttackRaw(entity, 0x1810E8F8);
    entity->hit.flags &= ~2;
    entity->hit.flags |= 4;
}

/**
 * @brief 6F750 | Player_ApplyContactDamage with (attack, 2, 0, 3) at the entity
 * position, then Player_SetKnockbackVelocity with (+-2 px by the facing, -5 px).
 */
void WerewolfShockwave_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;
    s32 knockbackX;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 2;
    contactDamage.unk_4 = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    contactDamage.reaction = 3;
    Player_ApplyContactDamage(&contactDamage);
    knockbackX = -0x20000;
    if (entity->flipX)
        knockbackX = -knockbackX;
    Player_SetKnockbackVelocity(knockbackX, -0x50000);
}

/**
 * @brief 6F7A4 | Werewolf create: load the graphics (delete the enemy
 * when that fails), start the animation and set up the hit boxes; state 4
 * instead when bestiaryPreview is set.
 */
void EnemyWerewolfCreate(struct EwramData_EntityData *entity)
{
    if (!Werewolf_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        return;
    }
    Hit_EnableContact(entity, 8, Werewolf_DamagePlayerOnContact, (s32)Werewolf_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = 0;
}
