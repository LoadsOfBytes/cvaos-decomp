/* SkillGreatArmorUse: Great Armor soul effect. Local register bindings in the
   stat multiply are compiler constraints, not evidence of original spelling. */
#include "code_08009A0.h"
#include "blue_soul_entity.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
#include "macros.h"
#define GRAPHICS_CONTEXT ((struct PlayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, playerState)))
extern u8 sUnk_081E116C[], sUnk_0820B64C[], sUnk_0823760C[];
extern s32 BlueSoul_ShouldEnd(struct EwramData_EntityData *entity);

/* Inlined Entity_PlaceRelativeTo (src/code_08031D6C.c). */
static inline void FollowParent(struct EwramData_EntityData *entity, struct EwramData_EntityData *anchor, u16 offsetX, u16 offsetY)
{
    s16 dx = offsetX;
    s16 dy = offsetY;

    entity->flipX = anchor->flipX;
    if (entity->flipX)
        dx = -dx;
    entity->posX.part16.integer = (s32)dx + (s16)anchor->posX.part16.integer;
    entity->posY.part16.integer = (s32)dy + anchor->posY.part16.integer;
}

void SkillGreatArmorUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = BLUE_SOUL_OWNER(entity);
    s16 *stats = gEwramData->playerState.currentStats;
    u32 t;

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = sUnk_081E116C;
        u8 *palette = sUnk_0820B64C;
        u8 *sprite = sUnk_0823760C;
        GRAPHICS_CONTEXT->blueSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
        ObjPalette_LoadRange(palette, 0, 1, 3);
        Sprite_Attach(entity, sprite, GRAPHICS_CONTEXT->blueSoulGfx, 3);
    }
        Anim_StartIndexed(entity, 1, 0, 1);
        entity->spriteLayer = 0xD;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->state++;
    case 1:
        FollowParent(entity, parent, 0, 0);
        Anim_Update(entity);
        t = entity->timer + 1;
        entity->timer = t;
        if ((t & 0x1F) == 0)
            PlaySong(0x94);
        else if ((t & 0xF) == 0 && (RandomNumberGenerator() & 1))
            PlaySong(0x94);
        if ((entity->timer & 7) == 0)
        {
            s32 x = parent->posX.whole + (RandomSigned(0x10) << 16);
            Effect_SpawnShared(x, parent->posY.whole + ((RandomSigned(0x10) - 0x10) << 16), 0x10, 7);
        }
        break;
    }
    stats += BLUE_SOUL_VALUE(entity);
    { register s32 product asm("r0") = *stats * 0x133; /* FAKEMATCH: pinned register */
      register s32 result asm("r1"); /* FAKEMATCH: pinned register */
      if (product < 0) product += 255;
      result = product >> 8;
      { s16 *destination = stats;
      *destination = result; } }
    Player_RecalculateAttackDefense();
}
