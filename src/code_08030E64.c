#include "code_08009A0.h"
#include "soul_skill_entity.h"
#include "player_entity.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern s32 BlueSoul_ShouldEnd(struct EwramData_EntityData *);
extern void BlueSoul_ReleaseActiveResources(struct EwramData_EntityData *);
extern void SkillCatoblepasProjectileUpdate(struct EwramData_EntityData *);

static inline void Follow(struct EwramData_EntityData *entity, struct EwramData_EntityData *anchor, u16 offsetX, s32 offsetY)
{
    s16 dx = offsetX;

    entity->flipX = anchor->flipX;
    if (entity->flipX)
        dx = -dx;
    entity->posX.part16.integer = (s32)dx + (s16)anchor->posX.part16.integer;
    entity->posY.part16.integer = anchor->posY.part16.integer + offsetY;
}

extern s32 sSkillProjectileLaunchVelocities[];
static inline void LoadGfx(struct EwramData_EntityData *entity, u8 paletteSlot, void *gfxSrc, void *palette, void *sheet)
{
    gEwramData->playerState.blueSoulGfx = ObjGfx_LoadResource(gfxSrc, 0x10000);
    ObjPalette_LoadRange(palette, paletteSlot, 1, 3);
    Sprite_Attach(entity, sheet, gEwramData->playerState.blueSoulGfx, 3);
}

/**
 * @brief 30E64 | Catoblepas blue soul, same frame as SkillBonePillarUse:
 * graphics 0x081E2978 / 0x0820B6B8 / 0x08237884, Hit_SetTimerOverride(2, 30),
 * skillBaseAttack/skillAttackAttributes = 10 / 0x100. Every 5 frames breathes a SkillCatoblepasProjectileUpdate
 * projectile (song 0x8E every other one) at the head, speed 0x18000 +
 * caster velX, deceleration 0x400, vertical speed sSkillProjectileLaunchVelocities[unk_50A]
 * + 0x8000 and vertical acceleration 0x200.
 */
void SkillCatoblepasUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    if ((parent->ext.player.actionFlags & 0x1001A2) != 0x100000)
    {
        BlueSoul_ReleaseActiveResources(entity);
        EntityDelete(entity);
        return;
    }
    gEwramData->playerState.actionLockFlags |= 0x820;
    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = (u8 *)0x081E2978;
        u8 *palette = (u8 *)0x0820B6B8;
        u8 *sprite = (u8 *)0x08237884;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        entity->hidden = 1;
        Hit_SetTimerOverride(2, 0x1E);
        {
            u16 a = 0xA;
            u16 b = 0x100;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        gEwramData->playerState.player->state = 0xA;
        Anim_StartIndexed(gEwramData->playerState.player, 0x35, 3, 1);
        parent->ext.player.actionFlags &= 0xEFFFFBFF;
        entity->state++;
    case 1:
        Follow(entity, parent, 0x10, -0x1C);
        if (entity->timer == 0)
        {
            struct EwramData_EntityData *child = BlueSoul_CreateEntity(entity, SkillCatoblepasProjectileUpdate);
            if (child != NULL)
            {
                s32 vx, vy;
                if (entity->subState)
                    PlaySong(0x8E);
                entity->subState ^= 1;
                Follow(child, entity, 0, 0);
                vx = 0x18000;
                child->velX.whole = vx;
                vy = 0x400;
                child->accelX.whole = vy;
                if (child->flipX)
                {
                    child->velX.whole = -vx;
                    child->accelX.whole = -vy;
                }
                child->velX.whole += parent->velX.whole;
                child->flipX = parent->flipX;
                child->velY.whole = sSkillProjectileLaunchVelocities[entity->ext.skillCatoblepas.shotIndex] + 0x8000;
                child->accelY.whole = 0x200;
            }
            if (++entity->ext.skillCatoblepas.shotIndex > 2)
                entity->ext.skillCatoblepas.shotIndex = 0;
            entity->timer = 5;
        }
        entity->timer--;
    }
}
