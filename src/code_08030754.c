#include "code_08009A0.h"
#include "soul_skill_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "macros.h"

#include "structs/entity_render_flags.h"

#define GRAPHICS_CONTEXT ((struct PlayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, playerState)))
#define TO_PIXELS(x) ((x) < 0 ? -(-(x) >> 16) : (x) >> 16)

extern u8 sUnk_080E1AF8[];
extern s16 sUnk_080E1AFC[];
extern u8 sUnk_081E116C[], sUnk_0820B64C[], sUnk_0823760C[];
extern s32 AngleTurnDirection(u16, u16);

static inline void FreeMatrix(struct EwramData_EntityData *entity)
{
    if (ENTITY_RENDER_FLAGS16(entity) & 0x101F)
        ObjAffine_Free(entity->affineIndex);
}

static inline void Destroy(struct EwramData_EntityData *entity)
{
    gEwramData->playerState.activeBlueSoulEntities[entity->ext.blueSoul.slot] = NULL;
    EntityDelete(entity);
}

/* Blue soul segment: turns towards its pose angle and orbits its parent. */
/**
 * @brief Body segment of the Creaking Skull soul: turn towards its pose angle
 * and orbit its parent.
 */
void SkillCreakingSkullSegmentUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *soul = &gEwramData->entityData[SKILL_CREAKING_SKULL_SOUL_INDEX(entity)];
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    u16 *angles = (u16 *)SKILL_CREAKING_SKULL_ANGLES(soul);
    u16 count = *angles;
    u16 matrix;
    s32 radius;
    u16 angle;
    s16 a;

    angles += 2;
    entity->roomActive = 1;
    if (entity->state != 0 && SKILL_CREAKING_SKULL_ACTIVE(soul) != 0)
        entity->state = 2;
    entity->flipX = soul->flipX;
    switch (entity->state)
    {
    case 0:
        matrix = ObjAffine_Alloc(0);
        if ((s16)matrix < 0)
        {
            FreeMatrix(entity);
            /* Dead store; it keeps a block boundary after the call so that
             * cross-jumping merges this copy with the one in case 4. */
            radius = 0;
            goto remove;
        }
        {
            u8 *graphics = sUnk_081E116C + 0x804;
            u8 *palette = sUnk_0820B64C + 0x24;
            u8 *sprite = sUnk_0823760C + 0x70;
            GRAPHICS_CONTEXT->blueSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
            ObjPalette_LoadRange(palette, 0, 1, 3);
            Sprite_Attach(entity, sprite, GRAPHICS_CONTEXT->blueSoulGfx, 3);
        }
        entity->animFrame = sUnk_080E1AF8[SKILL_CREAKING_SKULL_SEGMENT(entity)];
        entity->spriteLayer = 0xF;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->affineIndex = matrix;
        entity->affine = 1;
        entity->ext.skillCreakingSkullSegment.unused = 0x200000;
        SKILL_CREAKING_SKULL_SEGMENT_ANGLE(entity) = angles[SKILL_CREAKING_SKULL_SEGMENT(entity)];
        entity->scrollWithBg = 1;
        goto idle;
    case 2:
    {
        s16 delta;
        a = angles[SKILL_CREAKING_SKULL_SEGMENT(entity)];
        AngleTurnDirection(SKILL_CREAKING_SKULL_SEGMENT_ANGLE(entity), a);
        delta = a - SKILL_CREAKING_SKULL_SEGMENT_ANGLE(entity);
        SKILL_CREAKING_SKULL_SEGMENT_TURN(entity) = delta / (s16)count;
        entity->spawnParam1 = count;
        entity->state = 3;
    }
    case 3:
    {
        a = angles[SKILL_CREAKING_SKULL_SEGMENT(entity)];
        SKILL_CREAKING_SKULL_SEGMENT_ANGLE(entity) += SKILL_CREAKING_SKULL_SEGMENT_TURN(entity);
        if (entity->spawnParam1 != 0)
        {
            entity->spawnParam1--;
            break;
        }
        SKILL_CREAKING_SKULL_SEGMENT_ANGLE(entity) = a;
    idle:
        entity->state = 1;
        break;
    }
    case 1:
        break;
    case 4:
        if (ENTITY_RENDER_FLAGS16(entity) & 0x101F)
            ObjAffine_Free(entity->affineIndex);
    remove:
        Destroy(entity);
        return;
    }
    if (SKILL_CREAKING_SKULL_SEGMENT(entity) != 0)
    {
        radius = TO_PIXELS(sUnk_080E1AFC[SKILL_CREAKING_SKULL_SEGMENT(entity)] * soul->ext.skillCreakingSkull.scale);
        angle = -0x8000 + SKILL_CREAKING_SKULL_ANGLE(parent);
        if (entity->flipX)
            angle = -SKILL_CREAKING_SKULL_ANGLE(parent);
        entity->posX.whole = parent->posX.whole + radius * Sin((s16)angle + 0x4000);
        entity->posY.whole = parent->posY.whole + radius * Sin((s16)angle);
    }
    else
    {
        entity->posX.whole = parent->posX.whole;
        entity->posY.whole = parent->posY.whole;
    }
    angle = SKILL_CREAKING_SKULL_SEGMENT_ANGLE(entity);
    if (entity->flipX)
        angle = -(angle << 16) >> 16; /* negated in the upper half */
    ObjAffine_Set((s16)angle, soul->ext.skillCreakingSkull.scale, soul->ext.skillCreakingSkull.scale, entity->flipX, 0, entity->affineIndex);
}
