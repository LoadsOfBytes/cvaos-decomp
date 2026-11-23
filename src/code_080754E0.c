/* Winged Skeleton helpers 0x080754E0..0x0807565F. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern void EntityDampVelocityX(struct EwramData_EntityData *entity, s32 factor);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *entity, s32 maxSpeed);
extern s32 Entity_GetVisibilityPattern(s32 step);

/* Graphics of the Winged Skeleton. */
extern u8 sWingedSkeletonGfx[];
extern u8 sWingedSkeletonPalette[];
extern u8 sWingedSkeletonSpriteSheet[];

void WingedSkeletonShot_DamagePlayerOnContact(struct EwramData_EntityData *entity);

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
 * @brief 754E0 | Load the graphics of the Winged Skeleton; 0 when no slot is free.
 */
u8 WingedSkeleton_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sWingedSkeletonGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sWingedSkeletonPalette, 0);
    Sprite_Attach(entity, sWingedSkeletonSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 7552C | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position
 * (same as Chaos2Part_DamagePlayerOnContact).
 */
void WingedSkeleton_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief 75558 | Give child the Winged Skeleton graphics (palette row of entity),
 * animation 8; 0 when no graphics slot is free.
 */
s32 WingedSkeletonShot_LoadGraphics(struct EwramData_EntityData *entity, struct EwramData_EntityData *child)
{
    s32 gfx = ObjGfx_Find(sWingedSkeletonGfx);

    if (gfx < 0)
        return 0;
    Sprite_Attach(child, sWingedSkeletonSpriteSheet, gfx, entity->basePalette);
    Anim_StartIndexed(child, 8, 0, 1);
    Anim_Update(child);
    child->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 755A8 | Register the hit box 0x0404FC00 (contact WingedSkeletonShot_DamagePlayerOnContact).
 */
void WingedSkeletonShot_EnableHitBoxes(struct EwramData_EntityData *entity)
{
    u32 box = 0x0404FC00;

    Hit_Enable(entity, 8, WingedSkeletonShot_DamagePlayerOnContact, 0);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags &= ~2;
    entity->hit.flags |= 4;
}

/**
 * @brief 755E4 | Contact: Player_ApplyContactDamage with (attack, 0, 0, 1).
 */
void WingedSkeletonShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief Death piece: slow down, fall, blink during the last 0x28 frames and
 * delete after timer.
 */
void WingedSkeletonPiece_Update(struct EwramData_EntityData *entity)
{
    EntityDampVelocityX(entity, 8);
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->timer <= 0x28)
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
    if ((s8)--entity->timer <= 0)
        EntityDelete(entity);
}
