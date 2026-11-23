#include "code_08009A0.h"
#include "blue_soul_entity.h"
#include "player_entity.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern s32 BlueSoul_ShouldEnd(struct EwramData_EntityData *);
extern void BlueSoul_ReleaseActiveResources(struct EwramData_EntityData *);

struct BlueSoulInfo {
    u32 updateFunc;
    u32 mpPerSecond; /* u8 mpPerSecond, u8 activationFlags, u16 effectParam; unused here */
    u32 effectValue;
};

struct BlueSoulForm {
    void (*func)(struct EwramData_EntityData *);
    u16 baseAttack;
    u16 attackAttributes;
    u16 song;
    u16 pad_A;
};

static inline void Follow(struct EwramData_EntityData *entity, struct EwramData_EntityData *anchor, u16 offsetX, s32 offsetY)
{
    s16 dx = offsetX;

    entity->flipX = anchor->flipX;
    if (entity->flipX)
        dx = -dx;
    entity->posX.part16.integer = (s32)dx + (s16)anchor->posX.part16.integer;
    entity->posY.part16.integer = anchor->posY.part16.integer + offsetY;
}

extern struct BlueSoulInfo sBlueSoulInfo[];
extern struct BlueSoulForm sBlueSoulForms[];
/**
 * @brief 2EFB0 | Devil / Manticore / Curly blue soul (transformation; ends
 * via BlueSoul_ShouldEnd, or BlueSoul_ReleaseActiveResources while the player has unk_4F4 & 0x81A2):
 * follows the caster, keeps the player in state 9 and soulStateFlags bit 12.
 * State 0 stops the player and selects form sBlueSoulInfo[equipped blue
 * soul].unk_8 (player unk_50B); when the player reaches step 3 the form's
 * entity (sBlueSoulForms) is spawned, box 0x4430C0E8 in slot 6, skillBaseAttack/skillAttackAttributes
 * from the form, its song (0x19F if Player_IsBossTargetLowHp) and a white flash.
 */
void Skill_Devil_Manticore_Curly_Use(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = BLUE_SOUL_OWNER(entity);
    struct BlueSoulInfo *info = &sBlueSoulInfo[gEwramData->playerStatus.equippedBlueSoul];
    struct BlueSoulInfo *info2 = info;
    struct PlayerState *ctx;
    struct BlueSoulForm *form;

    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    ctx = &gEwramData->playerState;
    if (PLAYER_ACTION_FLAGS(ctx->player) & 0x81A2)
    {
        BlueSoul_ReleaseActiveResources(entity);
        EntityDelete(entity);
        return;
    }
    Follow(entity, parent, 0, 0);
    switch (entity->state)
    {
    case 0:
        ctx->player->accelX.whole = 0;
        ctx->player->accelY.whole = 0;
        PLAYER_ACTION_FLAGS(ctx->player) &= ~0x20;
        entity->state = 1;
        ctx->player->subState = 0;
        PLAYER_TRANSFORM_FORM(gEwramData->playerState.player) = info->effectValue;
        break;
    case 1:
        if (ctx->player->subState == 3)
        {
            form = &sBlueSoulForms[info2->effectValue];
            BlueSoul_CreateEntity(parent, form->func);
            Hit_Enable(entity, 6, 0, 0);
            HitBox_SetAttackRaw(entity, 0x4430C0E8);
            entity->hit.flags |= 4;
            entity->hit.flags &= ~2;
            Hit_SetTimerOverride(2, 0xF);
            {
                u16 a = form->baseAttack;
                u16 b = form->attackAttributes;
                gEwramData->playerState.skillBaseAttack = a;
                gEwramData->playerState.skillAttackAttributes = b;
            }
            if (Player_IsBossTargetLowHp())
                PlaySong(0x19F);
            else
                PlaySong(form->song);
            entity->state++;
            BgPalette_StartColorFade(0x1F, 0xF, 0xF, 0x8000, 0x10000, 0x10);
        }
        break;
    case 2:
        if (!BgPalette_GetPendingFades())
            BgPalette_StopColorFade();
        gEwramData->playerState.actionLockFlags |= 0xE;
        entity->timer++;
        break;
    }
    gEwramData->playerState.player->state = 9;
    gEwramData->playerStatus.soulStateFlags |= 0x1000;
}
