/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   EnemyLightningDollCreate = EnemyKillerDollCreate */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 LightningDoll_LoadGraphics(struct EwramData_EntityData *entity);
extern void LightningDoll_ResetActive(struct EwramData_EntityData *entity);
extern void LightningDoll_OnContact(struct EwramData_EntityData *entity);
extern void LightningDoll_OnHit(struct EwramData_EntityData *param_0);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_EnableBoxes(struct EwramData_EntityData *entity, void *onHit, s32 onHurt);

/**
 * @brief C89B0 | Same as EnemyKillerDollCreate.
 */
void EnemyLightningDollCreate(struct EwramData_EntityData *entity)
{
    if (!LightningDoll_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->state = 0;
    entity->stage = 0;
    entity->subState = 0;
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 9;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
        return;
    }
    Entity_EnableBoxes(entity, LightningDoll_OnContact, (s32)LightningDoll_OnHit);
    LightningDoll_ResetActive(entity);
}

/* Graphics loader 0x080C8A18. It only matches without -f2003-patch (see
 * NO_2003_PATCH_SRCS in ../Makefile). */
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/* Graphics of the Lightning Doll (shared with the Killer Doll). */
extern u8 sKillerDollLightningDollGfx[];
extern u8 sKillerDollLightningDollPalette[];
extern u8 sLightningDollSpriteSheet[];

/**
 * @brief Find or load graphics and palette, attach the sprite and start animation 0; return 0 on failure, 1 on success.
 */
s32 LightningDoll_LoadGraphics(struct EwramData_EntityData *entity)
{
    u8 *graphics = sKillerDollLightningDollGfx;
    s32 gfx = ObjGfx_Find(graphics);
    s8 paletteIndex;

    if (gfx < 0)
    {
        gfx = ObjGfx_LoadResource(graphics, 0);
        if (gfx < 0)
            return 0;
    }
    paletteIndex = ObjPalette_FindOrLoadVariant(sKillerDollLightningDollPalette, 0);
    if (paletteIndex == -1)
        return 0;
    entity->unk_510.part8.basePalette = paletteIndex;
    if (!Sprite_Attach(entity, sLightningDollSpriteSheet, gfx, entity->unk_510.part8.basePalette))
        return 0;
    if (!Anim_StartIndexed(entity, 0, 0, 1))
        return 0;
    Anim_Update(entity);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
