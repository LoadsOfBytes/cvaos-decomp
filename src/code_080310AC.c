#include "code_08009A0.h"
#include "soul_skill_entity.h"
#include "blue_soul_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern s32 BlueSoul_CheckEndCondition(struct EwramData_EntityData *);
extern void BlueSoul_ReleaseActiveResources(struct EwramData_EntityData *);
extern void Anim_UpdateWithFrameAttackBox(struct EwramData_EntityData *);
extern struct EwramData_EntityData *Player_GetSelectedOrNearestTarget(struct EwramData_EntityData *);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 Entity_StepTowardPoint(struct EwramData_EntityData *, s32, s32, s32);
extern s32 Entity_DistanceToPoint(struct EwramData_EntityData *, s32, s32);

static inline void LoadGfx(struct EwramData_EntityData *entity, u8 paletteSlot, void *gfxSrc, void *palette, void *sheet)
{
    gEwramData->playerState.blueSoulGfx = ObjGfx_LoadResource(gfxSrc, 0x10000);
    ObjPalette_LoadRange(palette, paletteSlot, 1, 3);
    Sprite_Attach(entity, sheet, gEwramData->playerState.blueSoulGfx, 3);
}
/**
 * @brief 310AC | Imp blue soul (familiar, ends via BlueSoul_CheckEndCondition): graphics
 * 0x081E317C / palette 0x0820B6DC / sprite 0x08237AC4 (slot 3), layer 0xF,
 * box slot 6, Hit_SetTimerOverride(2, 60), skillBaseAttack/skillAttackAttributes = 15 / 0x11, song 0x19E.
 * Rises from the caster, then flies towards its anchor point
 * (Target_GetX / Target_GetY - 40 px), circles it (sin/cos by timer)
 * and now and then (1-2 in 256) picks the nearest target (Player_GetSelectedOrNearestTarget) to
 * attack at a random height of 16-23 px above it; state 6 waits 16 frames
 * and ends via BlueSoul_ReleaseActiveResources.
 */

void SkillImpUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = BLUE_SOUL_OWNER(entity);
    s32 x = Target_GetX(entity);
    s32 y = Target_GetY(entity) - 0x280000;
    s32 dist;

    entity->roomActive = 1;
    {
    u8 six = 6;
    if (entity->state < six && BlueSoul_CheckEndCondition(entity))
    {
        entity->state = six;
        entity->drawSkip = 1;
        entity->timer = 0x10;
    }
    }
    switch (entity->state)
    {
    case 0:
    {
        u8 *graphics = (u8 *)0x081E317C;
        u8 *palette = (u8 *)0x0820B6DC;
        u8 *sprite = (u8 *)0x08237AC4;
        LoadGfx(entity, 0, graphics, palette, sprite);
    }
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->spriteLayer = 0xF;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->scrollWithBg = 1;
        entity->flipX = parent->flipX;
        entity->posX.part16.integer = parent->posX.part16.integer;
        entity->posY.part16.integer = parent->posY.part16.integer - 0x10;
        entity->velY.whole = -0x4000;
        Hit_Enable(entity, 6, 0, 0);
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        Hit_SetTimerOverride(2, 0x3C);
        {
            u16 a = 0xF;
            u16 b = 0x11;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        entity->state = 1;
        PlaySong(0x19E);
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->state = 2;
        break;
    case 2:
        if ((RandomNumberGenerator() & 0xFF) == 0)
        {
            entity->state = 4;
            break;
        }
        dist = Entity_StepTowardPoint(entity, x, y, 5);
        if (entity->velX.whole >= 0)
            entity->flipX = 1;
        else
            entity->flipX = 0;
        if (dist <= 0x7FFF)
            entity->state = 3;
        break;
    case 3:
        if ((RandomNumberGenerator() & 0xFF) <= 1)
        {
            entity->state = 4;
            break;
        }
        entity->timer++;
        dist = entity->timer << 8;
        entity->velX.whole = Sin(dist) / 4;
        dist = entity->timer << 9;
        entity->velY.whole = Sin(dist + 0xC000) / 4;
        EntityAccelerateAndMove(entity, 0x20000);
        if (Entity_DistanceToPoint(entity, x, y) > 0x300000)
            entity->state = 2;
        break;
    case 4:
        SKILL_IMP_TARGET(entity) = Player_GetSelectedOrNearestTarget(entity);
        if (SKILL_IMP_TARGET(entity) == NULL)
        {
            entity->state = 2;
            entity->timer = 0x80;
            break;
        }
        SKILL_IMP_AIM_Y(entity) = -(((RandomNumberGenerator() & 7) + 0x10) << 16);
        entity->state = 5;
    case 5:
    {
        struct EwramData_EntityData *target = SKILL_IMP_TARGET(entity);
        s32 timer = entity->timer - 1;
        entity->timer = timer;
        if ((s16)target->unk_518.part16.hp > 0 && (u8)timer != 0)
        {
            dist = Entity_StepTowardPoint(entity, target->posX.whole, target->posY.whole + SKILL_IMP_AIM_Y(entity), 4);
            if (entity->velX.whole >= 0)
                entity->flipX = 1;
            else
                entity->flipX = 0;
            if (dist > 0x3FFF)
                break;
        }
        entity->state = 2;
        break;
    }
    case 6:
        if (entity->timer == 0)
        {
            BlueSoul_ReleaseActiveResources(entity);
            EntityDelete(entity);
            return;
        }
        entity->timer--;
        break;
    }
    if (entity->state != 6)
        Anim_UpdateWithFrameAttackBox(entity);
}
