#include "code_08009A0.h"
#include "soul_skill_entity.h"
#include "player_entity.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern s32 BlueSoul_ShouldEnd(struct EwramData_EntityData *);

static inline void LoadGfx(struct EwramData_EntityData *entity, u8 paletteSlot, void *gfxSrc, void *palette, void *sheet)
{
    gEwramData->playerState.blueSoulGfx = ObjGfx_LoadResource(gfxSrc, 0x10000);
    ObjPalette_LoadRange(palette, paletteSlot, 1, 3);
    Sprite_Attach(entity, sheet, gEwramData->playerState.blueSoulGfx, 3);
}
extern void SkillCreakingSkullSegmentUpdate(struct EwramData_EntityData *);
extern void SkillCreakingSkullHitBoxUpdate(struct EwramData_EntityData *);

struct SkullFrame {
    u8 duration;
    u8 pad_1;
    u16 frame;
};

/**
 * @brief 30A24 | Creaking Skull blue soul (ends via BlueSoul_ShouldEnd): graphics
 * 0x081E1970 / 0x0820B670 / 0x0823767C, hidden, a chain of four children
 * (three SkillCreakingSkullSegmentUpdate segments and the SkillCreakingSkullHitBoxUpdate head, each spawned from
 * the previous one, spawnParam0 = caster byte / index), Hit_SetTimerOverride(2, 30),
 * skillBaseAttack/skillAttackAttributes = 0x28 / 0. Grows, then plays frame scripts (hp: idle
 * 0x084F13DC, bite 0x084F13E8 while the caster's weaponEntities slot is taken;
 * entries {duration, frame}, NULL ends; song 0x9B on bite step 3) 20 px
 * above the caster.
 */
void SkillCreakingSkullUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    struct EwramData_EntityData *prev, *child;
    struct SkullFrame *frame;
    s32 i;

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    SKILL_CREAKING_SKULL_ACTIVE(entity) = 0;
    if (entity->stage != 0 && gEwramData->playerState.weaponEntities[parent->ext.player.weaponSlot] != NULL)
    {
        SKILL_CREAKING_SKULL_FRAMES(entity) = 0x084F13E8;
        entity->state = 3;
        entity->stage = 0;
    }
    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = (u8 *)0x081E1970;
        u8 *palette = (u8 *)0x0820B670;
        u8 *sprite = (u8 *)0x0823767C;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        entity->hidden = 1;
        prev = entity;
        for (i = 0; i < 4; i++)
        {
            if (i < 3)
                child = BlueSoul_CreateEntity(prev, SkillCreakingSkullSegmentUpdate);
            else
                child = BlueSoul_CreateEntity(prev, SkillCreakingSkullHitBoxUpdate);
            if (child != NULL)
            {
                SKILL_CREAKING_SKULL_SOUL_INDEX(child) = ENTITY_INDEX(entity);
                SKILL_CREAKING_SKULL_SEGMENT(child) = i;
                prev = child;
            }
        }
        Hit_SetTimerOverride(2, 0x1E);
        {
            u16 a = 0x28;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = 0;
        }
        SKILL_CREAKING_SKULL_ANGLES(entity) = 0x080E1AA8;
        SKILL_CREAKING_SKULL_FRAMES(entity) = 0x084F13DC;
        entity->flipX = 1 ^ parent->flipX;
        entity->state = 1;
    case 1:
        if (entity->ext.skillCreakingSkull.scale <= 0xFFFF)
            entity->ext.skillCreakingSkull.scale += 0x1000;
        else
            entity->state = 2;
        break;
    case 2:
        SKILL_CREAKING_SKULL_FRAMES(entity) = 0x084F13DC;
        entity->state = 3;
        entity->stage = 1;
        break;
    case 3:
        {
            struct SkullFrame **list = (struct SkullFrame **)SKILL_CREAKING_SKULL_FRAMES(entity);
            frame = list[entity->subState];
        }
        if (frame == NULL)
        {
            entity->state = 2;
            entity->subState = 0;
            break;
        }
        SKILL_CREAKING_SKULL_ANGLES(entity) = (s32)frame;
        SKILL_CREAKING_SKULL_ACTIVE(entity) = 1;
        entity->timer = frame->duration - 1;
        SKILL_CREAKING_SKULL_FRAME(entity) = frame->frame;
        entity->state = 4;
    case 4:
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        if (SKILL_CREAKING_SKULL_FRAMES(entity) == 0x084F13E8 && entity->subState == 3)
            PlaySong(0x9B);
        entity->state = 3;
        entity->subState++;
        break;
    }
    entity->posX.part16.integer = parent->posX.part16.integer;
    entity->posY.part16.integer = parent->posY.part16.integer - 0x14;
}
