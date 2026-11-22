#include "code_08009A0.h"
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
    gEwramData->playerState.activeBlueSoulEntities[entity->unk_508.part16.unk_508] = NULL;
    EntityDelete(entity);
}

/* Blue soul segment: turns towards its pose angle and orbits its parent. */
/**
 * @brief Body segment of the Creaking Skull soul: turn towards its pose angle
 * and orbit its parent.
 */
void SkillCreakingSkullSegmentUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *soul = &gEwramData->entityData[entity->unk_514.part8.unk_514];
    struct EwramData_EntityData *parent = entity->unk_4F8.unk_4F8_entity;
    u16 *angles = (u16 *)soul->unk_510.whole;
    u16 count = *angles;
    u16 matrix;
    s32 radius;
    u16 angle;
    s16 a;

    angles += 2;
    entity->roomActive = 1;
    if (entity->state != 0 && soul->unk_514.part8.unk_514 != 0)
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
        entity->animFrame = sUnk_080E1AF8[entity->unk_514.part8.unk_515];
        entity->spriteLayer = 0xF;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->affineIndex = matrix;
        entity->affine = 1;
        entity->unk_500.whole = 0x200000;
        entity->unk_510.part16.unk_510 = angles[entity->unk_514.part8.unk_515];
        entity->scrollWithBg = 1;
        goto idle;
    case 2:
    {
        s16 delta;
        a = angles[entity->unk_514.part8.unk_515];
        AngleTurnDirection(entity->unk_510.part16.unk_510, a);
        delta = a - entity->unk_510.part16.unk_510;
        entity->unk_510.part16.unk_512 = delta / (s16)count;
        entity->unk_514.part16.spawnParam1 = count;
        entity->state = 3;
    }
    case 3:
    {
        a = angles[entity->unk_514.part8.unk_515];
        entity->unk_510.part16.unk_510 += entity->unk_510.part16.unk_512;
        if (entity->unk_514.part16.spawnParam1 != 0)
        {
            entity->unk_514.part16.spawnParam1--;
            break;
        }
        entity->unk_510.part16.unk_510 = a;
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
    if (entity->unk_514.part8.unk_515 != 0)
    {
        radius = TO_PIXELS(sUnk_080E1AFC[entity->unk_514.part8.unk_515] * soul->unk_50C.whole);
        angle = -0x8000 + parent->unk_510.part16.unk_510;
        if (entity->flipX)
            angle = -parent->unk_510.part16.unk_510;
        entity->posX.whole = parent->posX.whole + radius * Sin((s16)angle + 0x4000);
        entity->posY.whole = parent->posY.whole + radius * Sin((s16)angle);
    }
    else
    {
        entity->posX.whole = parent->posX.whole;
        entity->posY.whole = parent->posY.whole;
    }
    angle = entity->unk_510.part16.unk_510;
    if (entity->flipX)
        angle = -(angle << 16) >> 16; /* negated in the upper half */
    ObjAffine_Set((s16)angle, soul->unk_50C.whole, soul->unk_50C.whole, entity->flipX, 0, entity->affineIndex);
}
