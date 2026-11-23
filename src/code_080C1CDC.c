/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Altair_UpdateBestiaryPreview = Imp_UpdateState4Animation
 *   Altair_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "code/code_080211F0.h"

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
 * @brief Bestiary state (4): start animation 0 once, then update it.
 */
void Altair_UpdateBestiaryPreview(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState++;
        case 1:
            Anim_Update(entity);
            break;
    }
}

/**
 * @brief Apply the entity damage with zero effect type and flags at its position.
 */
void Altair_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/* Fleaman helpers, create and graphics loader 0x080C1D38..0x080C1F1B. The loader
 * Fleaman_LoadGraphics only matches without -f2003-patch. */
extern struct EwramData_EntityData *EntityCreateInRange(s32 minSlot, s32 maxSlot, u32 *updateFunc);
extern s32 ObjPalette_FindOrLoad(void *palette);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_EnableBoxes(struct EwramData_EntityData *entity, void *onHit, s32 onHurt);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern void AltairFeather_Update(struct EwramData_EntityData *entity);
extern void Fleaman_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void Fleaman_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
s32 AltairFeather_SetupGraphics(struct EwramData_EntityData *entity);
u8 AltairFeather_LoadGraphics(struct EwramData_EntityData *entity);
u8 Fleaman_LoadGraphics(struct EwramData_EntityData *entity);

/* Graphics of the Altair feather (AltairFeather_LoadGraphics) and of the Fleaman
 * (Fleaman_LoadGraphics). */
extern u8 sAltairGfx[];
extern u8 sAltairPalette[];
extern u8 sAltairSpriteSheet[];
extern u8 sFleamanRipperGfx[];
extern u8 sFleamanRipperPalette[];
extern u8 sFleamanRipperSpriteSheet[];

/**
 * @brief Allocate a feather, attach its graphics and store the owner pointer.
 * Return the feather or NULL; retain the existing failure cleanup calls.
 */
struct EwramData_EntityData *Altair_CreateFeather(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)AltairFeather_Update);

    if (child != NULL)
    {
        if (!AltairFeather_SetupGraphics(child))
        {
            EntityDelete(child);
            return NULL;
        }
        ENTITY_CREATOR(child) = entity;
    }
    return child;
}

/**
 * @brief Return whether the feather graphics loader succeeds.
 */
s32 AltairFeather_SetupGraphics(struct EwramData_EntityData *entity)
{
    return AltairFeather_LoadGraphics(entity) != 0;
}

/**
 * @brief Attach frame 9 using already-loaded Altair graphics and a palette.
 * Return 1 on success; delete the entity and return 0 on failure.
 */
u8 AltairFeather_LoadGraphics(struct EwramData_EntityData *entity)
{
    void *palette = sAltairPalette;
    void *sprite = sAltairSpriteSheet;
    s32 tileSlot = ObjGfx_Find(sAltairGfx);

    if (tileSlot >= 0)
    {
        entity->unk_510.part8.basePalette = ObjPalette_FindOrLoad(palette);
        if (entity->unk_510.part8.basePalette != 0xFF
            && Sprite_Attach(entity, sprite, tileSlot, entity->unk_510.part8.basePalette))
        {
            entity->animFrame = 9;
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            return 1;
        }
    }
    EntityDelete(entity);
    return 0;
}

/**
 * @brief Set the feather phase to zero or 0x8000 according to the supplied flag.
 */
void AltairFeather_SetPhase(struct EwramData_EntityData *entity, u8 phase)
{
    if (phase == 0)
        entity->ext.altairFeather.swayPhase = phase;
    else
        entity->ext.altairFeather.swayPhase = 0x8000;
}

/**
 * @brief C1E0C | Fleaman create: load the graphics, layer 0x14, state 1,
 * hit boxes; state 3 when bestiaryPreview is set. unk_4F8 = 1 when unk_514 and
 * unk_516 both have all low bits set (then no Entity_SnapToGround), collision
 * callbacks Fleaman_DamagePlayerOnContact / Fleaman_OnHit.
 */
void EnemyFleamanCreate(struct EwramData_EntityData *entity)
{
    if (!Fleaman_LoadGraphics(entity))
        return;
    entity->spriteLayer = 0x14;
    entity->state = 1;
    entity->stage = 0;
    entity->subState = 0;
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
        return;
    }
    if ((entity->unk_514.part16.spawnParam0 & entity->unk_514.part16.spawnParam1) == 0xFF)
        entity->ext.fleaman.airborne = 1;
    else
        entity->ext.fleaman.airborne = 0;
    Entity_EnableBoxes(entity, Fleaman_DamagePlayerOnContact, (s32)Fleaman_OnHit);
    if ((u8)entity->ext.fleaman.airborne != 1)
        Entity_SnapToGround(entity);
}

/**
 * @brief C1E88 | Load the Fleaman graphics (reusing an already loaded
 * copy), palette, sprite and hit boxes; deletes the entity and returns 0
 * when any step fails.
 */
u8 Fleaman_LoadGraphics(struct EwramData_EntityData *entity)
{
    void *palette = sFleamanRipperPalette;
    void *sprite = sFleamanRipperSpriteSheet;
    u8 *graphics = sFleamanRipperGfx;
    s32 gfx = ObjGfx_Find(graphics);
    s32 paletteIndex;

    if (gfx < 0)
    {
        gfx = ObjGfx_LoadResource(graphics, 0);
        if (gfx < 0)
            goto fail;
    }
    paletteIndex = (s8)ObjPalette_FindOrLoad(palette);
    if (paletteIndex == -1)
        goto fail;
    entity->unk_510.part8.basePalette = paletteIndex;
    if (!Sprite_Attach(entity, sprite, gfx, entity->unk_510.part8.basePalette))
        goto fail;
    if (!Anim_StartIndexed(entity, 0, 0, 1))
    {
        entity->ext.fleaman.anim = 0;
        goto fail;
    }
    Anim_Update(entity);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
fail:
    EntityDelete(entity);
    return 0;
}
