#include "code_08009A0.h"
#include "blue_soul_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
#include "macros.h"
#define GRAPHICS_CONTEXT ((struct PlayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, playerState)))
extern u8 sSkillFlyingArmorGfx[], sSkillFlyingArmorPalette[], sSkillFlyingArmorSpriteSheet[];
extern s32 BlueSoul_ShouldEnd(struct EwramData_EntityData *);

static inline void FollowParent(struct EwramData_EntityData *entity, struct EwramData_EntityData *parent, u16 x_, u16 y_)
{
    s16 x = x_, y = y_;
    entity->flipX = parent->flipX;
    if (entity->flipX) x = -x;
    entity->posX.part16.integer = (s32)x + (s16)parent->posX.part16.integer;
    entity->posY.part16.integer = (s32)y + parent->posY.part16.integer;
}

/* Flying Armor guardian: follows the owner and emits drifting particles. */
void SkillFlyingArmorUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = BLUE_SOUL_OWNER(entity);
    struct EwramData_EntityData *particle;
    u32 timer;
    s32 x, y, r;
    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = sSkillFlyingArmorGfx;
        u8 *palette = sSkillFlyingArmorPalette;
        u8 *sprite = sSkillFlyingArmorSpriteSheet;
        GRAPHICS_CONTEXT->blueSoulGfx = ObjGfx_LoadResource(graphics, 0x10000);
        ObjPalette_LoadRange(palette, 0, 1, 3);
        Sprite_Attach(entity, sprite, GRAPHICS_CONTEXT->blueSoulGfx, 3);
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->spriteLayer = 13;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->state++;
    case 1:
        FollowParent(entity, parent, 0, 0);
        Anim_Update(entity);
        break;
    }
    timer = entity->timer + 1;
    entity->timer = timer;
    if ((timer & 3) == 0)
    {
        x = parent->posX.whole + RandomSigned(0x80000);
        y = RandomSigned(0x10000) - 0x1A0000;
        particle = Effect_CreateFromTable(x, parent->posY.whole + y, 0, 0, 8, 9);
        if (particle != NULL)
        {
            particle->velX.whole = RandomSigned(0x8000);
            particle->velY.whole = RandomSigned(0x4000) - 0x10000;
            r = RandomSigned(0x1000);
            particle->accelX.whole = -particle->velX.whole / 4 + r;
            r = RandomSigned(0x1000);
            particle->accelY.whole = -particle->velY.whole / 64 + r;
        }
    }
    gEwramData->playerStatus.soulStateFlags |= 0x100;
}


