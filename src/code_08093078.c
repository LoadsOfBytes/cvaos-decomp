/* Zombie Soldier helpers 0x08093078..0x08093177. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *entity, s32 maxSpeed);

/* Graphics of Zombie Soldier. */
extern u8 sZombieSoldierGfx[];
extern u8 sZombieSoldierPalette[];
extern u8 sZombieSoldierSpriteSheet[];

static inline void Entity_DamagePlayerOnContact(struct EwramData_EntityData *entity, u16 attributes, u16 param_2, u16 reaction)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = attributes;
    contactDamage.unk_4 = param_2;
    contactDamage.reaction = reaction;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 93078 | Fall (max 0x80000); delete below y unk_500.
 */
void ZombieSoldierPiece_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->posY.whole > entity->unk_500.whole)
        EntityDelete(entity);
}

/**
 * @brief 93098 | Contact: Player_ApplyContactDamage(attack, 2, 0, 1); from state 0 go to
 * state 1 with sound 0x76.
 */
void ZombieSoldierGrenade_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 2;
    contactDamage.unk_4 = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    if (entity->state == 0)
    {
        entity->state = 1;
        entity->timer = 0;
        PlaySong(0x76);
    }
}

/**
 * @brief 930E0 | Hit: hit effect, sound 0x76, state 1.
 */
void ZombieSoldierGrenade_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Player_CalculateAttackPower(hit[2]);
    PlaySong(0x76);
    entity->state = 1;
    entity->subState = 0;
    entity->timer = 0;
}

/**
 * @brief 93100 | Load the graphics of Zombie Soldier; 0 when no slot is free.
 */
u8 ZombieSoldier_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sZombieSoldierGfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sZombieSoldierPalette, 0);
    Sprite_Attach(entity, sZombieSoldierSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 9314C | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position
 * (same as Chaos2Part_DamagePlayerOnContact).
 */
void ZombieSoldier_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
