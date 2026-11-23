/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   SkeletonProjectile_DamagePlayerOnContact = Object28_OnPlayerContact */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief Apply the Skeleton projectile damage with effect type 0 and
 * effect flags 1 at the entity position.
 */
void SkeletonProjectile_DamagePlayerOnContact(struct EwramData_EntityData *entity)
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

/* Tiny Devil spawn and create 0x080C46AC..0x080C4767. */
extern struct EwramData_EntityData *Entity_CreateObject(u32 *updateFunc);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_EnableBoxes(struct EwramData_EntityData *entity, void *onHit, s32 onHurt);
extern void Imp_OnContact(struct EwramData_EntityData *entity);
extern void Imp_ResetFlight(struct EwramData_EntityData *entity);
extern void Imp_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern s32 Imp_CreateGrabIndicator(struct EwramData_EntityData *entity);
extern s32 Imp_LoadGraphics(struct EwramData_EntityData *entity);

/**
 * @brief Create with the supplied update function, copy parent identity, attack and health fields, and hide the parent; NULL on allocation failure.
 */
struct EwramData_EntityData *Entity_CreateLinkedAndHide(struct EwramData_EntityData *entity, u32 *updateFunc)
{
    struct EwramData_EntityData *child = Entity_CreateObject(updateFunc);

    if (child == NULL)
        return NULL;
    child->ext.skeletonPiece.ownerIndex = ENTITY_INDEX_PAIR(entity);
    ENTITY_INDEX_PAIR(child) = ENTITY_INDEX_PAIR(entity);
    child->attack = entity->attack;
    child->unk_518.part16.hp = entity->unk_518.part16.hp;
    entity->hidden = 1;
    return child;
}

/**
 * @brief C46EC | Tiny Devil create: load the graphics (delete the enemy
 * when that fails), layer 4, hit boxes and Imp_ResetFlight; state 4 instead
 * when bestiaryPreview is set. Ends (0x53D bit 3) when Imp_CreateGrabIndicator fails.
 */
void EnemyImpCreate(struct EwramData_EntityData *entity)
{
    if (!Imp_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->state = 0;
    entity->stage = 0;
    entity->subState = 0;
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 4;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        entity->stage = 0;
        entity->subState = 0;
        return;
    }
    Entity_EnableBoxes(entity, Imp_OnContact, (s32)Imp_OnHit);
    Imp_ResetFlight(entity);
    if (!Imp_CreateGrabIndicator(entity))
        entity->deleteRequested = 1;
}

/* Graphics loader 0x080C4768. It only matches without -f2003-patch (see
 * NO_2003_PATCH_SRCS in ../Makefile). */
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/* Graphics of the Tiny Devil (also used in code_080C47F8.c). */
extern u8 sImpTinyDevilGfx[];
extern u8 sImpTinyDevilPalette[];
extern u8 sImpTinyDevilSpriteSheet[];

/**
 * @brief Find or load graphics and palette, attach the sprite and start animation 0; return 0 on failure, 1 on success.
 */
s32 Imp_LoadGraphics(struct EwramData_EntityData *entity)
{
    void *palette = sImpTinyDevilPalette;
    void *sprite = sImpTinyDevilSpriteSheet;
    u8 *graphics = sImpTinyDevilGfx;
    s32 gfx = ObjGfx_Find(graphics);
    s8 paletteIndex;

    if (gfx < 0)
    {
        gfx = ObjGfx_LoadResource(graphics, 0);
        if (gfx < 0)
            return 0;
    }
    paletteIndex = ObjPalette_FindOrLoadVariant(palette, 0);
    if (paletteIndex == -1)
        return 0;
    entity->unk_510.part8.basePalette = paletteIndex;
    if (!Sprite_Attach(entity, sprite, gfx, entity->unk_510.part8.basePalette))
        return 0;
    if (!Anim_StartIndexed(entity, 0, 0, 1))
        return 0;
    Anim_Update(entity);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
