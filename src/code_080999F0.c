/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Ectoplasm_LoadGraphics = Object25_LoadGraphics
 *
 * Ectoplasm_UpdatePreview..EctoplasmParticle_Update decompiled locally. */
#include "code_08009A0.h"
#include "ectoplasm_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sEctoplasmGfx[];
extern u8 sEctoplasmPalette[];
extern u8 sEctoplasmSpriteSheet[];
void Ectoplasm_Draw(struct EwramData_EntityData *entity);
void Ectoplasm_SpawnParticle(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY, s32 frame);
void EctoplasmParticle_Update(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *updateFunc);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *entity, s32 maxSpeed);
extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *entity, s32 margin);
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/**
 * @brief 999F0 | Same as Object25_LoadGraphics.
 */
u8 Ectoplasm_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sEctoplasmGfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sEctoplasmPalette, 0);
    Sprite_Attach(entity, sEctoplasmSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Ectoplasm_Draw;
    return 1;
}

/**
 * @brief 99A3C | Substate 0: follow the room position (unk_4F8, unk_4FC)
 * relative to background 1, bobbing vertically by 3 * cos(unk_500).
 */
void Ectoplasm_UpdatePreview(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 baseY;

    if (entity->subState == 0)
    {
        entity->posX.whole = ECTOPLASM_POINT_X(entity) - bg->xPos.whole;
        baseY = ECTOPLASM_POINT_Y(entity) - bg->yPos.whole;
        entity->posY.whole = baseY + Sin(ECTOPLASM_BOB_PHASE(entity) + 0x4000) * 3;
        ECTOPLASM_BOB_PHASE(entity) += 0x200;
        entity->timer++;
    }
}

/**
 * @brief 99A90 | Draw; while visible (and unless paused) spawn a particle
 * of type 3 or 9 at a random offset every 8 frames.
 */
void Ectoplasm_Draw(struct EwramData_EntityData *entity)
{
    s32 offsetX;
    s32 offsetY;

    Sprite_Draw(entity);
    if (gEwramData->paused && !gEwramData->gameState.bestiaryPreview)
        return;
    if (EntityIsOutsideScreen(entity, 6))
        return;

    offsetX = (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
    offsetY = -(RandomNumberGenerator() & 0x7FFFF);
    if ((gEwramData->frameCounter & 7) == 0)
        Ectoplasm_SpawnParticle(entity, offsetX, offsetY, 3);
    if ((gEwramData->frameCounter & 7) == 4)
        Ectoplasm_SpawnParticle(entity, offsetX, offsetY, 9);
}

/**
 * @brief 99B30 | Player_ApplyContactDamage with (attack, 0x80, 0, 0) at the entity
 * position.
 */
void Ectoplasm_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0x80;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 99B60 | Spawn a rising particle (update EctoplasmParticle_Update) at the
 * entity position plus (offsetX, offsetY) with frame frame.
 */
void Ectoplasm_SpawnParticle(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY, s32 frame)
{
    struct EwramData_EntityData *child = Entity_CreateObject((u32 *)EctoplasmParticle_Update);
    s32 gfx;

    if (child != NULL)
    {
        child->posX.whole = entity->posX.whole + offsetX;
        child->posY.whole = entity->posY.whole + offsetY;
        child->spriteLayer = entity->spriteLayer + 1;
        gfx = ObjGfx_Find(sEctoplasmGfx);
        Sprite_Attach(child, sEctoplasmSpriteSheet, gfx, entity->unk_510.part8.basePalette);
        child->animFrame = frame;
        child->flipX = RandomNumberGenerator() % 2;
        child->velY.whole = -0x10000;
        child->accelY.whole = -0x400;
        child->scrollWithBg = 1;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/**
 * @brief 99C08 | Particle update: rise, advance the frame every 3 frames
 * and delete at frame 9 or 13 or when off screen.
 */
void EctoplasmParticle_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMove(entity, 0x20000);
    if (entity->timer++ > 2)
    {
        entity->timer = 0;
        entity->animFrame++;
        if (entity->animFrame == 9 || entity->animFrame == 0xD)
        {
            EntityDelete(entity);
            return;
        }
    }
    if (EntityIsOutsideScreen(entity, 6))
        EntityDelete(entity);
}
