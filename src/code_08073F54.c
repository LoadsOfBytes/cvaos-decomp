/* Manticore helpers 0x08073F54..0x0807412F. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);
extern void Manticore_OnDamaged(struct EwramData_EntityData *entity, s32 attacker, u16 *hit, s32 damage);

/* Graphics of Manticore. */
extern u8 sManticoreGfx[];
extern u8 sManticorePalette[];
extern u8 sManticoreSpriteSheet[];

void ManticoreShot_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void ManticoreShot_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);


/**
 * @brief 73F54 | Give shot Manticore's graphics (palette row of entity),
 * animation 6; 0 when no graphics slot is free.
 */
s32 ManticoreShot_LoadGraphics(struct EwramData_EntityData *entity, struct EwramData_EntityData *shot)
{
    s32 gfx = ObjGfx_Find(sManticoreGfx);

    if (gfx < 0)
        return 0;
    Sprite_Attach(shot, sManticoreSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    Anim_StartIndexed(shot, 6, 0, 0);
    Anim_Update(shot);
    shot->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 73FA4 | Register the hit box (contact ManticoreShot_DamagePlayerOnContact, hit
 * ManticoreShot_OnHit) of size 0x1818F4F4.
 */
void ManticoreShot_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    Hit_Enable(entity, 10, ManticoreShot_DamagePlayerOnContact, (s32)ManticoreShot_OnHit);
    HitBox_SetBodyRaw(entity, 0x1818F4F4);
    HitBox_SetAttackRaw(entity, 0x1818F4F4);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
}

/**
 * @brief 73FE4 | Contact: Player_ApplyContactDamage(attack, 2, 0, 1), then go to state 2
 * with sound 0x76.
 */
void ManticoreShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
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
    entity->subState = 0;
    entity->timer = 0;
    PlaySong(0x76);
}

/**
 * @brief 74028 | Hit: hit effect, then go to state 2 with sound 0x76.
 */
void ManticoreShot_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Player_CalculateAttackPower(hit[2]);
    entity->state = 2;
    entity->subState = 0;
    entity->timer = 0;
    PlaySong(0x76);
}

/**
 * @brief 74048 | Delete the entity when 0x53D bit 0 is set, else animate
 * (same as Chaos2Effect_Update).
 */
void ManticoreEffect_Update(struct EwramData_EntityData *entity)
{
    if (entity->animDone)
        EntityDelete(entity);
    else
        Anim_Update(entity);
}

/**
 * @brief 7406C | Load the graphics of Manticore; 0 when no slot is free.
 */
u8 Manticore_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sManticoreGfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sManticorePalette, 0);
    Sprite_Attach(entity, sManticoreSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 740B8 | Player_ApplyContactDamage with (attack / 2, 0, 0, 0) at the entity
 * position.
 */
void Manticore_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack >> 1;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 740E8 | Hit: damage from hit, damage number, then Manticore_OnDamaged.
 */
void Manticore_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s16 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    Entity_ShowDamage(entity, damage, 0);
    Manticore_OnDamaged(entity, attacker, hit, damage);
}
