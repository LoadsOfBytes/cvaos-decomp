/* White Dragon helpers 0x08077B3C..0x08077CBB (graphics shared with Quezlcoatl). */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *entity, s32 maxSpeed);
extern s32 Entity_GetVisibilityPattern(s32 step);
extern void WhiteDragon_Draw(struct EwramData_EntityData *entity);

/* Graphics of Quezlcoatl. */
extern u8 sQuezlcoatlWhiteDragonGfx[];
extern u8 sQuezlcoatlWhiteDragonPalette[];
extern u8 sQuezlcoatlWhiteDragonSpriteSheet[];

/**
 * @brief 77B3C | Give projectile the White Dragon / Quezlcoatl graphics (palette row of
 * entity), frame 5; 0 when no graphics slot is free.
 */
s32 WhiteDragonBreath_LoadGraphics(struct EwramData_EntityData *entity, struct EwramData_EntityData *projectile)
{
    s32 gfx = ObjGfx_Find(sQuezlcoatlWhiteDragonGfx);

    if (gfx < 0)
        return 0;
    Sprite_Attach(projectile, sQuezlcoatlWhiteDragonSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    projectile->animFrame = 5;
    projectile->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 77B80 | Player_ApplyContactDamage with (attack, 2, 0, 1) at the entity position.
 */
void WhiteDragonBreath_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 2;
    contactDamage.unk_4 = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 77BB0 | Random flips each frame; delete after 9 frames.
 */
void WhiteDragonSpark_Update(struct EwramData_EntityData *entity)
{
    entity->flipX = RandomNumberGenerator() % 2;
    entity->flipY = RandomNumberGenerator() % 2;
    if (entity->timer++ > 8)
        EntityDelete(entity);
}

/**
 * @brief 77BFC | Fall (max 0x70000), blink, delete after timer.
 */
void WhiteDragonPiece_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMove(entity, 0x70000);
    entity->hidden = Entity_GetVisibilityPattern(entity->timer);
    if ((s8)entity->timer-- <= 0)
        EntityDelete(entity);
}

/**
 * @brief 77C40 | Load the White Dragon graphics (palette 0 of the set shared with Quezlcoatl) (drawn by WhiteDragon_Draw);
 * 0 when no slot is free.
 */
u8 WhiteDragon_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sQuezlcoatlWhiteDragonGfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sQuezlcoatlWhiteDragonPalette, 0);
    Sprite_Attach(entity, sQuezlcoatlWhiteDragonSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)WhiteDragon_Draw;
    return 1;
}

/**
 * @brief 77C8C | Player_ApplyContactDamage with (attack, 0, 0, 1) at the entity position.
 */
void WhiteDragon_DamagePlayerOnContact(struct EwramData_EntityData *entity)
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
