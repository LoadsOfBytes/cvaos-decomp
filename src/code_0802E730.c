#include "code_08009A0.h"
#include "soul_skill_entity.h"
#include "blue_soul_entity.h"
#include "player_entity.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern s32 BlueSoul_ShouldEnd(struct EwramData_EntityData *);
extern void BlueSoul_ReleaseActiveResources(struct EwramData_EntityData *);
extern void SkillBonePillarProjectileUpdate(struct EwramData_EntityData *);

static inline void Follow(struct EwramData_EntityData *entity, struct EwramData_EntityData *anchor, u16 offsetX, s32 offsetY)
{
    s16 dx = offsetX;

    entity->flipX = anchor->flipX;
    if (entity->flipX)
        dx = -dx;
    entity->posX.part16.integer = (s32)dx + (s16)anchor->posX.part16.integer;
    entity->posY.part16.integer = anchor->posY.part16.integer + offsetY;
}

static inline void LoadGfx(struct EwramData_EntityData *entity, u8 paletteSlot, void *gfxSrc, void *palette, void *sheet)
{
    gEwramData->playerState.blueSoulGfx = ObjGfx_LoadResource(gfxSrc, 0x10000);
    ObjPalette_LoadRange(palette, paletteSlot, 1, 3);
    Sprite_Attach(entity, sheet, gEwramData->playerState.blueSoulGfx, 3);
}

extern s32 sSkillProjectileLaunchVelocities[];
/**
 * @brief 2E730 | Bone Pillar blue soul (ends via BlueSoul_ShouldEnd or when the
 * caster leaves the 0x100000 state of unk_4F4 & 0x1001A2): graphics
 * 0x081DC948 / 0x0820B508 / 0x082362B8 (slot 3), hidden, Hit_SetTimerOverride(2,
 * 15), skillBaseAttack/skillAttackAttributes = 0x14 / 2, the player (playerState.player) to state 10 with
 * animation 0x35. Sits 16 px in front of and 28 px above the caster and
 * every 5 frames throws a SkillBonePillarProjectileUpdate bone (song 0x91 on throw 0 and 50 %
 * on throw 2 of 5) with speed 0x18000 + caster velX, deceleration 0x400 and
 * vertical speed sSkillProjectileLaunchVelocities[unk_50A] + 0x14000 (unk_50A cycles 0..2).
 */
void SkillBonePillarUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = BLUE_SOUL_OWNER(entity);

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    if ((PLAYER_ACTION_FLAGS(parent) & 0x1001A2) != 0x100000)
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
        u8 *graphics = (u8 *)0x081DC948;
        u8 *palette = (u8 *)0x0820B508;
        u8 *sprite = (u8 *)0x082362B8;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        entity->hidden = 1;
        Hit_SetTimerOverride(2, 0xF);
        {
            u16 a = 0x14;
            u16 b = 2;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        gEwramData->playerState.player->state = 0xA;
        Anim_StartIndexed(gEwramData->playerState.player, 0x35, 3, 1);
        PLAYER_ACTION_FLAGS(parent) &= 0xEFFFFBFF;
        entity->state++;
    case 1:
        Follow(entity, parent, 0x10, -0x1C);
        if (entity->timer == 0)
        {
            struct EwramData_EntityData *child = BlueSoul_CreateEntity(entity, SkillBonePillarProjectileUpdate);
            if (child != NULL)
            {
                if (entity->subState == 0)
                    PlaySong(0x91);
                else if (entity->subState == 2 && (RandomNumberGenerator() & 1))
                    PlaySong(0x91);
                if (++entity->subState > 4)
                    entity->subState = 0;
                ENTITY_OWNER(child) = entity;
                child->posX.whole = entity->posX.whole;
                child->posY.whole = entity->posY.whole;
                ENTITY_INDEX(child) = ENTITY_INDEX(entity);
                if (parent->flipX)
                {
                    child->velX.whole = -0x18000;
                    child->accelX.whole = -0x400;
                }
                else
                {
                    child->velX.whole = 0x18000;
                    child->accelX.whole = 0x400;
                }
                child->velX.whole += parent->velX.whole;
                child->flipX = parent->flipX;
                child->velY.whole = sSkillProjectileLaunchVelocities[SKILL_BONE_PILLAR_SHOT(entity)] + 0x14000;
                child->accelY.whole = -0x1800;
            }
            if (++SKILL_BONE_PILLAR_SHOT(entity) > 2)
                SKILL_BONE_PILLAR_SHOT(entity) = 0;
            entity->timer = 5;
        }
        entity->timer--;
    }
}
