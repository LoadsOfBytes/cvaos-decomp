/* Merman helpers 0x080717E0..0x08071903. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern u16 Player_CalculateAttackPower(u16 attackCategory);

/* Graphics of Merman. */
extern u8 sMermanGfx[];
extern u8 sMermanPalette[];
extern u8 sMermanSpriteSheet[];

static inline void Entity_DamagePlayerOnContact(struct EwramData_EntityData *entity, u16 attributes, u16 unused, u16 reaction)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = attributes;
    contactDamage.unused = unused;
    contactDamage.reaction = reaction;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 717E0 | Give projectile Merman's graphics (palette row of entity),
 * animation 6; 0 when no graphics slot is free.
 */
s32 MermanShot_LoadGraphics(struct EwramData_EntityData *entity, struct EwramData_EntityData *projectile)
{
    s32 gfx = ObjGfx_Find(sMermanGfx);

    if (gfx < 0)
        return 0;
    Sprite_Attach(projectile, sMermanSpriteSheet, gfx, entity->basePalette);
    Anim_StartIndexed(projectile, 6, 0, 1);
    Anim_Update(projectile);
    projectile->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 71830 | Contact: Player_ApplyContactDamage(attack, 2, 0, 1), then state 2 with a
 * 0x28 frame timer.
 */
void MermanShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 2;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    entity->state = 2;
    entity->timer = 0x28;
    entity->subState = 0;
}

/**
 * @brief 71870 | Hit: hit effect, then state 2 with a 0x28 frame timer.
 */
void MermanShot_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Player_CalculateAttackPower(hit[2]);
    entity->state = 2;
    entity->timer = 0x28;
    entity->subState = 0;
}

/**
 * @brief 7188C | Load the graphics of Merman; 0 when no slot is free.
 */
u8 Merman_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sMermanGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sMermanPalette, 0);
    Sprite_Attach(entity, sMermanSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 718D8 | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position
 * (same as Chaos2Part_DamagePlayerOnContact).
 */
void Enemy_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
