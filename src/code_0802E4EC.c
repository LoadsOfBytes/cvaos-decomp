#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
extern s32 BlueSoul_ShouldEnd(struct EwramData_EntityData *);

#include "macros.h"
#define GRAPHICS_CONTEXT ((struct PlayerState *)((u8 *)gEwramData + OFFSET_OF(struct EwramData, playerState)))
extern u8 sSkillCagnazzoGfx[], sSkillCagnazzoPalette[], sSkillCagnazzoSpriteSheet[];
static inline void FollowParent(struct EwramData_EntityData *entity, struct EwramData_EntityData *parent, u16 x_, s32 y_)
{
    s16 x = x_;
    s32 y = y_;
    entity->flipX = parent->flipX;
    if (entity->flipX) x = -x;
    entity->posX.part16.integer = (s32)x + (s16)parent->posX.part16.integer;
    entity->posY.part16.integer = (s32)y + parent->posY.part16.integer;
}

void SkillCagnazzoUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.blueSoul.owner;

    u8 state;

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    switch (state = entity->state)
    {
    case 0:
    {
        u8 *gfx = sSkillCagnazzoGfx;
        u8 *pal = sSkillCagnazzoPalette;
        void *oam = sSkillCagnazzoSpriteSheet;
        GRAPHICS_CONTEXT->blueSoulGfx = ObjGfx_LoadResource(gfx, 0x10000);
        ObjPalette_LoadRange(pal, 0, 1, 3);
        Sprite_Attach(entity, oam, GRAPHICS_CONTEXT->blueSoulGfx, 3);
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->spriteLayer = 0xF;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        {
            u16 n = 0xC;
            GRAPHICS_CONTEXT->skillBaseAttack = n;
        }
        GRAPHICS_CONTEXT->skillAttackAttributes = state;
        entity->scrollWithBg = 1;
        entity->state++;
        Hit_Enable(entity, 6, 0, 0);
        HitBox_SetAttackRaw(entity, 0x1414050A);
        entity->hit.flags |= 4;
        Hit_SetTimerOverride(2, 0x14);
        PlaySong(0x1AA);
    }
    case 1:
        FollowParent(entity, owner, 0x10, -0x20);
    }
    Anim_Update(entity);
}




