/* Animated blue-soul effect; release its affine and active-entity slots
 * once the animation finishes. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "macros.h"

#include "structs/entity_render_flags.h"

#define GRAPHICS_CONTEXT ((struct PlayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, playerState)))
extern u8 sSkillDevilManticoreSparkGfx[], sSkillDevilManticoreSparkPalette[], sSkillDevilManticoreSparkSpriteSheet[];
static inline void ReleaseAffineSlot(struct EwramData_EntityData *entity)
{
    if (ENTITY_RENDER_FLAGS16(entity) & 0x101F) ObjAffine_Free(entity->affineIndex);
}
/**
 * @brief Single spark spawned by SkillDevilManticoreSparksUpdate: random
 * animation, moves and deletes itself (freeing its affine slot).
 */
void SkillDevilManticoreSparkUpdate(struct EwramData_EntityData *entity)
{
    if (entity->state == 0)
    {
        u8 *graphics = sSkillDevilManticoreSparkGfx;
        u8 *palette = sSkillDevilManticoreSparkPalette;
        u8 *sprite = sSkillDevilManticoreSparkSpriteSheet;
        GRAPHICS_CONTEXT->blueSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
        ObjPalette_LoadRange(palette, 0, 1, 3);
        Sprite_Attach(entity, sprite, GRAPHICS_CONTEXT->blueSoulGfx, 3);
        Anim_StartIndexed(entity, RandomNumberGenerator() & 3, 0, 0);
        entity->spriteLayer = 13;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->state++;
    }
    entity->roomActive = 1;
    if (Anim_Update(entity) == 3)
    {
        ReleaseAffineSlot(entity);
        gEwramData->playerState.activeBlueSoulEntities[entity->ext.blueSoul.slot] = NULL;
        EntityDelete(entity);
    }
}
