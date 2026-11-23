/* Tsuchinoko helpers 0x080A79D8..0x080A7B53. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *entity, s32 maxSpeed);
extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *entity, s32 margin);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern u16 Player_CalculateAttackPower(u16 attackCategory);

/* Graphics of Tsuchinoko. */
extern u8 sTsuchinokoGfx[];
extern u8 sTsuchinokoPalette[];
extern u8 sTsuchinokoSpriteSheet[];

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
 * @brief A79D8 | Falling projectile with a hit box (Hit_CheckWithPlayer); deleted
 * outside the screen.
 */
void TsuchinokoShot_Update(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 0)
    {
        EntityAccelerateAndMove(entity, 0x40000);
        if (EntityIsOutsideScreen(entity, 1))
        {
            EntityDelete(entity);
            return;
        }
    }
    Hit_CheckWithPlayer(collision, entity);
    Anim_Update(entity);
}

/**
 * @brief A7A18 | Give child Tsuchinoko's graphics (palette row of
 * entity); 0 when no graphics slot is free.
 */
s32 TsuchinokoShot_LoadGraphics(struct EwramData_EntityData *entity, struct EwramData_EntityData *child)
{
    s32 gfx = ObjGfx_Find(sTsuchinokoGfx);

    if (gfx < 0)
        return 0;
    Sprite_Attach(child, sTsuchinokoSpriteSheet, gfx, entity->basePalette);
    child->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief A7A54 | Contact: Player_ApplyContactDamage(attack, 0x40, 0, 0), then end
 * (0x53D bit 3).
 */
void TsuchinokoShot_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0x40;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    entity->deleteRequested = 1;
}

/**
 * @brief A7A90 | Hit: hit effect and end (0x53D bit 3).
 */
void TsuchinokoShot_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    Player_CalculateAttackPower(hit[2]);
    entity->deleteRequested = 1;
}

/**
 * @brief A7AAC | Falling projectile without hit box; deleted outside the
 * screen.
 */
void TsuchinokoDroplet_Update(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 0)
    {
        EntityAccelerateAndMove(entity, 0x40000);
        if (EntityIsOutsideScreen(entity, 1))
            EntityDelete(entity);
    }
}

/**
 * @brief A7ADC | Load the graphics of Tsuchinoko; 0 when no slot is free.
 */
u8 Tsuchinoko_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sTsuchinokoGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sTsuchinokoPalette, 0);
    Sprite_Attach(entity, sTsuchinokoSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief A7B28 | Player_ApplyContactDamage with (attack, 0, 0, 0) at the entity position
 * (same as Chaos2Part_DamagePlayerOnContact).
 */
void Tsuchinoko_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
