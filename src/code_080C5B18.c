/* TinyDevil_UpdateState3Animation: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief Start animation 1 once, then advance it. The gameplay purpose of state 3 remains unresolved.
 */
void TinyDevil_UpdateState3Animation(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        Anim_StartIndexed(entity, 1, 0, 1);
        entity->subState++;
    }
    Anim_Update(entity);
}

/* Ukoback helpers and create from 0x080C5B40. */
extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void ObjAffine_Free(s32 index);
extern s32 Ukoback_LoadGraphics(struct EwramData_EntityData *entity);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_EnableBoxes(struct EwramData_EntityData *entity, void *onHit, s32 onHurt);
extern void Entity_FaceTarget(struct EwramData_EntityData *entity);
extern void Ukoback_OnContact(struct EwramData_EntityData *entity);
extern void Ukoback_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

/**
 * @brief Apply damage and reset to death state 0xFF on lethal damage.
 */
void TinyDevil_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        entity->state = 0xFF;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief Submit the attack contact record, free the affine slot and request projectile deletion.
 */
void TinyDevilProjectile_OnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    ObjAffine_Free(entity->affineIndex);
    entity->deleteRequested = 1;
}

/**
 * @brief Return 1 outside the inclusive inset rectangle of the 240x160 screen, otherwise 0; the inset is supplied in pixels.
 */
s32 Entity_IsNearScreenEdge(struct EwramData_EntityData *entity, s32 margin)
{
    if ((s16)entity->posX.part16.integer >= margin && (s16)entity->posX.part16.integer <= 240 - margin
        && (s16)entity->posY.part16.integer >= margin && (s16)entity->posY.part16.integer <= 160 - margin)
        return 0;
    return 1;
}

/**
 * @brief C5BDC | Ukoback create: load the graphics (delete the enemy when
 * that fails), hit boxes, layer 4; state 1 when bestiaryPreview is set,
 * otherwise collision callbacks, stopped, Entity_FaceTarget and state 0.
 */
void EnemyUkobackCreate(struct EwramData_EntityData *entity)
{
    if (!Ukoback_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 4;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 1;
        entity->stage = 0;
        entity->subState = 0;
        return;
    }
    Entity_EnableBoxes(entity, Ukoback_OnContact, (s32)Ukoback_OnHit);
    entity->accelX.whole = 0;
    entity->velX.whole = 0;
    entity->accelY.whole = 0;
    entity->velY.whole = 0;
    entity->ext.ukoback.fireballCount = 0;
    Entity_FaceTarget(entity);
    entity->state = 0;
    entity->stage = 0;
    entity->subState = 0;
}

/* Graphics loader 0x080C5C58. It only matches without -f2003-patch (see
 * NO_2003_PATCH_SRCS in ../Makefile). */
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/* Graphics of the Ukoback (also used in code_080C5F10.c). */
extern u8 sUkobackGfx[];
extern u8 sUkobackPalette[];
extern u8 sUkobackSpriteSheet[];

/**
 * @brief Find or load graphics and palette, attach the sprite and start animation 0; return 0 on failure, 1 on success.
 */
s32 Ukoback_LoadGraphics(struct EwramData_EntityData *entity)
{
    void *palette = sUkobackPalette;
    void *sprite = sUkobackSpriteSheet;
    u8 *graphics = sUkobackGfx;
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
