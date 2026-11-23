/* Great Armor body-part helpers 0x0808E94C..0x0808EB3B (same code as the Big Golem helpers in
 * code_080ACEC4.c with 0x28-byte parts). */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

#define GREAT_ARMOR_PART_COUNT 14

extern void GreatArmor_UpdatePart(struct EwramData_EntityData *entity, s32 index);
extern void Effect_UpdateParticle(struct EwramData_EntityData *entity);
extern void GreatArmorSlashEffect_Update(struct EwramData_EntityData *entity);
extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];

/**
 * @brief 8E94C | Unless timer is 0, run GreatArmor_UpdatePart on parts 0..13.
 */
void GreatArmor_UpdateAllParts(struct EwramData_EntityData *entity)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < GREAT_ARMOR_PART_COUNT; i++)
            GreatArmor_UpdatePart(entity, i);
    }
}

/**
 * @brief 8E96C | Unless timer is 0, run GreatArmor_UpdatePart on the 14 parts
 * listed in order.
 */
void GreatArmor_UpdateParts(struct EwramData_EntityData *entity, s8 *order)
{
    s32 i;

    if (entity->timer != 0)
    {
        for (i = 0; i < GREAT_ARMOR_PART_COUNT; i++)
            GreatArmor_UpdatePart(entity, order[i]);
    }
}

/**
 * @brief 8E994 | Clear unk_C and unk_10 of part partIndex.
 */
void GreatArmor_ResetPartAngles(struct EwramData_EntityData *entity, s32 partIndex)
{
    struct GreatArmorPart *parts = entity->ext.greatArmor.parts;

    parts[partIndex].targetSwingAngle = 0;
    parts[partIndex].swingAngle = 0;
}

/**
 * @brief 8E9A8 | Save the offsets of all parts.
 */
void GreatArmor_SaveParts(struct EwramData_EntityData *entity)
{
    struct GreatArmorPart *parts = entity->ext.greatArmor.parts;
    s32 i;

    for (i = GREAT_ARMOR_PART_COUNT - 1; i >= 0; parts++, i--)
    {
        parts->savedOffsetX = parts->offsetX;
        parts->savedOffsetY = parts->offsetY;
    }
}

/**
 * @brief 8E9C4 | Restore the offsets saved by GreatArmor_SaveParts.
 */
void GreatArmor_RestoreParts(struct EwramData_EntityData *entity)
{
    struct GreatArmorPart *parts = entity->ext.greatArmor.parts;
    s32 i;

    for (i = GREAT_ARMOR_PART_COUNT - 1; i >= 0; parts++, i--)
    {
        parts->offsetX = parts->savedOffsetX;
        parts->offsetY = parts->savedOffsetY;
    }
}

/**
 * @brief 8E9E0 | Player_ApplyContactDamage with (attack, 2, 0, 0) at the entity position.
 */
void GreatArmorShockwave_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 2;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 8EA10 | Spawn a Effect_UpdateParticle effect (animation 0xF) at (x,
 * y) moving with (velocityX, velocityY) on layer layer.
 */
void GreatArmor_SpawnParticle(s32 x, s32 y, s32 velocityX, s32 velocityY, s32 layer)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateParticle);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->velX.whole = velocityX;
    child->velY.whole = velocityY;
    child->spriteLayer = layer;
    child->timer = 0;
    child->ext.particle.flags = 3;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, 7);
    Anim_StartIndexed(child, 0xF, 0, 0);
    Anim_Update(child);
    child->scrollWithBg = 1;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
}

/**
 * @brief 8EA98 | Spawn a GreatArmorSlashEffect_Update effect (animation 0x10) following
 * entity at the offset of its part 12 (saved in accelX/accelY).
 */
void GreatArmorSlashEffect_Spawn(struct EwramData_EntityData *entity, s32 x, s32 y, s32 velocityX, s32 velocityY, s32 layer)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)GreatArmorSlashEffect_Update);
    struct GreatArmorPart *parts = entity->ext.greatArmor.parts;
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->velX.whole = velocityX;
    child->velY.whole = velocityY;
    // "*&" keeps the original address arithmetic (base + 0x1E0, base + 0x1E4)
    child->accelX.whole = *&parts[12].offsetX;
    child->accelY.whole = *&parts[12].offsetY;
    child->spriteLayer = layer;
    child->timer = 0;
    child->ext.greatArmorSlash.unusedKind = 3;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, 7);
    Anim_StartIndexed(child, 0x10, 0, 0);
    Anim_Update(child);
    child->ext.greatArmorSlash.owner = entity;
    child->scrollWithBg = 0;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
}
