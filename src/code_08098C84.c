/* Creaking Skull helpers 0x08098C84..0x08098F03 (Giant Skeleton uses the
 * Creaking Skull functions). */
#include "code_08009A0.h"
#include "joint_entity.h"
#include "creaking_skull_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);
extern void CreakingSkull_ApplyResolvedDamage(struct EwramData_EntityData *entity, s32 unused1, u16 *unused2, s16 damage);
extern void CreakingSkullGuidedEffect_Update(struct EwramData_EntityData *entity);
extern void EnemyCreakingSkullUpdate(struct EwramData_EntityData *entity);
extern void EnemyCreakingSkullCreate(struct EwramData_EntityData *entity);

/* Graphics of Creaking Skull and of the effect. */
extern u8 sCreakingSkullGfx[];
extern u8 sCreakingSkullPalette[];
extern u8 sCreakingSkullSpriteSheet[];
extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];

/**
 * @brief Resolve root damage at the struck part position while restoring
 * root coordinates, display damage on that part and pass it to the
 * root resolved-damage helper.
 */
void CreakingSkullPart_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    struct EwramData_EntityData *skull = JOINT_PART_OWNER(entity);
    s32 oldX = skull->posX.whole;
    s32 oldY = skull->posY.whole;
    s32 damage;

    skull->posX.whole = entity->posX.whole;
    skull->posY.whole = entity->posY.whole;
    damage = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(JOINT_PART_OWNER(entity), attacker, hit, (s16)damage);
    skull->posX.whole = oldX;
    skull->posY.whole = oldY;
    Entity_ShowDamage(entity, damage, 0);
    CreakingSkull_ApplyResolvedDamage(JOINT_PART_OWNER(entity), attacker, hit, damage);
}

/**
 * @brief Create a frame-0x66 effect at the supplied position with two
 * stored BG1-relative room targets, owner facing and countdown 20.
 * Exact visual identity remains unresolved.
 */
void CreakingSkull_SpawnGuidedEffect(struct EwramData_EntityData *entity, s32 lateTargetX, s32 lateTargetY, s32 earlyTargetX, s32 earlyTargetY, s32 x, s32 y)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)CreakingSkullGuidedEffect_Update);
    s32 gfx;

    if (child == NULL)
        return;
    child->posX.whole = x;
    child->posY.whole = y;
    child->spriteLayer = entity->spriteLayer - 1;
    CREAKING_SKULL_GUIDE_LATE_X(child) = lateTargetX + bg->xPos.whole;
    CREAKING_SKULL_GUIDE_LATE_Y(child) = lateTargetY + bg->yPos.whole;
    CREAKING_SKULL_GUIDE_EARLY_X(child) = bg->xPos.whole + earlyTargetX;
    CREAKING_SKULL_GUIDE_EARLY_Y(child) = earlyTargetY + bg->yPos.whole;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    Sprite_Attach(child, sCommonObjectSpriteSheet, gfx, 7);
    child->animFrame = 0x66;
    child->flipX = entity->flipX;
    child->timer = 0x14;
    child->scrollWithBg = 1;
    child->drawFunc = (u32 *)Sprite_DrawEntity;
}

/**
 * @brief Apply stored damage with effect type 0 and flags 1, then
 * select burst state 1 and play sound 0x76.
 */
void CreakingSkullProjectile_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unk_4 = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    entity->state = 1;
    PlaySong(0x76);
}

/**
 * @brief Select burst state 1 and play sound 0x76.
 */
void CreakingSkullProjectile_OnHit(struct EwramData_EntityData *entity)
{
    entity->state = 1;
    PlaySong(0x76);
}

/**
 * @brief Load shared graphics and select palette variant 0 for enemy
 * 0x21 or 1 for 0x5C. Return 0 on graphics-slot failure, otherwise
 * attach the sprite, set its draw callback and return 1.
 */
u8 CreakingSkull_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sCreakingSkullGfx, 0);

    if (gfx < 0)
        return 0;
    if (entity->unk_518.part8.enemyId == 0x21)
        entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sCreakingSkullPalette, 0);
    else if (entity->unk_518.part8.enemyId == 0x5C)
        entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sCreakingSkullPalette, 1);
    Sprite_Attach(entity, sCreakingSkullSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief Apply root contact damage with effect type 0 and flags 1.
 * Distinct from the joint-part CreakingSkull_OnContact callback.
 */
void CreakingSkull_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unk_4 = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief Resolve and display root body damage, then apply it through
 * the shared resolved-damage helper.
 */
void CreakingSkull_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s16 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    Entity_ShowDamage(entity, damage, 0);
    CreakingSkull_ApplyResolvedDamage(entity, attacker, hit, damage);
}

/**
 * @brief 98EEC | Giant Skeleton update: same as Creaking Skull.
 */
void EnemyGiantSkeletonUpdate(struct EwramData_EntityData *entity)
{
    EnemyCreakingSkullUpdate(entity);
}

/**
 * @brief 98EF8 | Giant Skeleton create: same as Creaking Skull.
 */
void EnemyGiantSkeletonCreate(struct EwramData_EntityData *entity)
{
    EnemyCreakingSkullCreate(entity);
}
