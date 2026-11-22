/* Shared Cockatrice/Basilisk death and creation.
 * Decompiled locally; patrol and attack helpers are now matching C. */
#include "code_08009A0.h"
#include "cockatrice_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Sound_StopTracked(s32 song);
extern void PlaySong(u16 song);
extern s32 RandomSigned(s32 range);
extern struct EwramData_EntityData *Effect_SpawnShared(s32 x, s32 y, s32 script, s32 palette);
extern s32 Entity_GetVisibilityPattern(s32 step);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, u8 variant, void *sheet);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *updateFunc);
extern s32 Hit_Enable(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);
extern void Enemy_OnContact(struct EwramData_EntityData *entity);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern void CockatriceFamilyPart_Update(struct EwramData_EntityData *entity);
extern void CockatriceFamily_OnHit(struct EwramData_EntityData *entity);

/* Graphics of the Cockatrice/Basilisk (see code_080BB7E4.c). */
extern u8 sCockatriceFamilyGfx[];
extern u8 sCockatriceFamilyPalette[];
extern u8 sCockatriceFamilySpriteSheet[];

/**
 * @brief Death state (2): stop tracked sound 0x65, animate with sparks and
 * palette flashes, then set the attached part termination flag. Return
 * nonzero on the following substate; preserve the stored-position arithmetic.
 */
s32 CockatriceFamily_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 wave;
    s32 jitter;
    struct EwramData_EntityData *spark;

    switch (entity->subState)
    {
        case 0:
            Sound_StopTracked(0x65);
            Anim_StartIndexed(entity, 2, 0, 1);
            entity->timer = 0x40;
            entity->subState = 1;
        case 1:
            entity->velY.whole -= 0x6000;
            entity->velX.whole += 0x3A00;
            if (!(entity->timer & 7))
                PlaySong(0x6E);
            if (!(entity->timer & 1))
            {
                wave = Sin(entity->velX.whole + 0x4000);
                jitter = RandomSigned(0x40000);
                spark = Effect_SpawnShared(entity->posX.whole + wave * 24 + jitter, entity->posY.whole + entity->velY.whole, 1, 7);
                if (spark != NULL)
                {
                    spark->velY.whole = 0x8000;
                    spark->accelY.whole = -0x1000;
                }
            }
            if (Entity_GetVisibilityPattern(entity->timer))
                entity->palette = 5;
            else
                entity->palette = entity->unk_510.part8.basePalette;
            if (entity->timer != 0)
            {
                entity->timer--;
                break;
            }
            entity->subState = 2;
            COCKATRICE_DEATH_FLAGS(entity) |= 1;
            break;
        case 2:
            return 1;
    }
    return 0;
}

/**
 * @brief Create Cockatrice (palette 0) or Basilisk (palette 1), including
 * the shared affine part. Bestiary preview uses state 3; gameplay sets
 * hit callbacks and snaps to the ground. Delete on graphics failure.
 */
void CockatriceFamily_Create(struct EwramData_EntityData *entity, s32 paletteSlot)
{
    struct EwramData_EntityData *part;

    if (!Entity_LoadGraphics(entity, sCockatriceFamilyGfx, sCockatriceFamilyPalette, paletteSlot, sCockatriceFamilySpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    COCKATRICE_PALETTE_SLOT(entity) = paletteSlot;
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x11;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    part = Entity_CreateObject((u32 *)CockatriceFamilyPart_Update);
    if (part != NULL)
    {
        ENTITY_OWNER(part) = entity;
        part->scrollWithBg = 1;
    }
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        return;
    }
    Hit_Enable(entity, 8, Enemy_OnContact, (s32)CockatriceFamily_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->posY.part16.integer++;
    entity->state = 0;
}
