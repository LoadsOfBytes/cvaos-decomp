#include "code_08009A0.h"
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
 * the previous one, unk_514 = caster byte / index), Hit_SetTimerOverride(2, 30),
 * skillBaseAttack/skillAttackAttributes = 0x28 / 0. Grows, then plays frame scripts (unk_518: idle
 * 0x084F13DC, bite 0x084F13E8 while the caster's weaponEntities slot is taken;
 * entries {duration, frame}, NULL ends; song 0x9B on bite step 3) 20 px
 * above the caster.
 */
void SkillCreakingSkullUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->unk_4F8.unk_4F8_entity;
    struct EwramData_EntityData *prev, *child;
    struct SkullFrame *frame;
    s32 i;

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->unk_514.part8.unk_514 = 0;
    if (entity->stage != 0 && gEwramData->playerState.weaponEntities[parent->unk_50C.part8.unk_50C] != NULL)
    {
        entity->unk_518.whole = 0x084F13E8;
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
                child->unk_514.part8.unk_514 = entity->unk_4F4.part8.unk_4F4;
                child->unk_514.part8.unk_515 = i;
                prev = child;
            }
        }
        Hit_SetTimerOverride(2, 0x1E);
        {
            u16 a = 0x28;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = 0;
        }
        entity->unk_510.whole = 0x080E1AA8;
        entity->unk_518.whole = 0x084F13DC;
        entity->flipX = 1 ^ parent->flipX;
        entity->state = 1;
    case 1:
        if (entity->unk_50C.whole <= 0xFFFF)
            entity->unk_50C.whole += 0x1000;
        else
            entity->state = 2;
        break;
    case 2:
        entity->unk_518.whole = 0x084F13DC;
        entity->state = 3;
        entity->stage = 1;
        break;
    case 3:
        {
            struct SkullFrame **list = (struct SkullFrame **)entity->unk_518.whole;
            frame = list[entity->subState];
        }
        if (frame == NULL)
        {
            entity->state = 2;
            entity->subState = 0;
            break;
        }
        entity->unk_510.whole = (s32)frame;
        entity->unk_514.part8.unk_514 = 1;
        entity->timer = frame->duration - 1;
        entity->unk_514.part8.unk_515 = frame->frame;
        entity->state = 4;
    case 4:
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        if (entity->unk_518.whole == 0x084F13E8 && entity->subState == 3)
            PlaySong(0x9B);
        entity->state = 3;
        entity->subState++;
        break;
    }
    entity->posX.part16.integer = parent->posX.part16.integer;
    entity->posY.part16.integer = parent->posY.part16.integer - 0x14;
}
