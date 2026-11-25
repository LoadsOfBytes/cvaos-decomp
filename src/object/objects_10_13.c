/*
 * Boss rush result and room objects 0x10-0x13 (Object 0x13 arm segments).
 */
#include "objects_10_13.h"
#include "core.h"
#include "in_game.h"
#include "graphics.h"
#include "syscalls.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
#include "gameplay.h"
#include "save_data.h"
#include "entity_util.h"
#include "player_core.h"
#include "sound.h"
#include "entity_hit.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "pickup.h"
#include "soul_pickup.h"
#include "data/data_0E5364.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "entity_inline.h"
#include "entity_common.h"

void BossRushTimer_Update(struct EwramData_EntityData *entity);
void RoomDecorationVariant2_Draw(struct EwramData_EntityData *entity);

/* BossRushResult_Draw: matched (three block-local v instead of one variable set three times;
   literal constants, no helper variables). */

/**
 * @brief Draw function of the result object (0x10): call Game_SetMenusEnabled(0), keep
 * the player input set in state 3 and, unless hidden, draw the labels, the
 * record frame when the timer (entity 26) beats the stored best bestBossRushTimeFrames or
 * none is stored, and the time with hundredths, seconds and minutes.
 */
void BossRushResult_Draw(struct EwramData_EntityData *entity)
{
    struct GameState *g;
    struct EwramData_EntityData *t;
    u32 time;
    g = &gEwramData->gameState;
    t = &gEwramData->entityData[26];
    time = t->ext.bossRushTimer.time;
    Game_SetMenusEnabled(0);
    if (entity->state == 3)
        SetPlayerInput(1, 0);
    if (*(ENTITY_RENDER_FLAGS_ADDRESS(entity)) & 0x20)
        return;
    entity->animFrame = 1;

    entity->posX.part16.integer = 0x78;
    entity->posY.part16.integer = 0x20;
    Sprite_Draw(entity);
    entity->animFrame = 0;
    entity->posY.part16.integer = 0x38;
    Sprite_Draw(entity);
    if (g->bestBossRushTimeFrames > time || g->bestBossRushTimeFrames == 0)
    {
        entity->animFrame = 2;
        entity->posY.part16.integer = 0x78;
        Sprite_Draw(entity);
    }
    entity->posY.part16.integer = 0x50;
    {
    s32 v = Div(Mod(time, 60) * 100, 60);
    entity->posX.part16.integer = 0x98;
    entity->animFrame = Mod(v, 10) + 3;
    Sprite_Draw(entity);
    entity->posX.part16.integer -= 8;
    entity->animFrame = Div(v, 10) + 3;
    Sprite_Draw(entity);

    entity->animFrame = 0xD;
    entity->posX.part16.integer -= 8;
    Sprite_Draw(entity);
    }
    {
    s32 v = Mod(Div(time, 60), 60);
    entity->posX.part16.integer = 0x80;
    entity->animFrame = Mod(v, 10) + 3;
    Sprite_Draw(entity);
    entity->posX.part16.integer -= 8;
    entity->animFrame = Div(v, 10) + 3;
    Sprite_Draw(entity);
    entity->animFrame = 0xD;
    entity->posX.part16.integer -= 8;
    Sprite_Draw(entity);
    }
    {
    s32 v = Div(time, 3600);
    entity->posX.part16.integer = 0x68;
    entity->animFrame = Mod(v, 10) + 3;
    Sprite_Draw(entity);
    entity->posX.part16.integer -= 8;
    entity->animFrame = Div(v, 10) + 3;
    Sprite_Draw(entity);
    }
}

void BossRushResultCreate(struct EwramData_EntityData *entity)
{
    u32 value;
    struct BossRushReward *reward;
    s32 i;
    s32 item;
    struct EwramData_EntityData *child;

    switch (gEwramData->gameState.language)
    {
    case 2:
        if (!Entity_LoadGraphics(entity, sBossRushResultGfxLanguage2, sBossRushResultPalette, 0, sBossRushResultSpriteSheetLanguage2))
        {
            EntityDelete(entity);
            return;
        }
    case 3:
        if (!Entity_LoadGraphics(entity, sBossRushResultGfxLanguage3, sBossRushResultPalette, 0, sBossRushResultSpriteSheetLanguage3))
        {
            EntityDelete(entity);
            return;
        }
        break;
    default:
        if (!Entity_LoadGraphics(entity, sBossRushResultGfx, sBossRushResultPalette, 0, sBossRushResultSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        break;
    }
    child = &gEwramData->entityData[26];
    value = child->ext.bossRushTimer.time;
    for (reward = sBossRushRewards, i = 0; i <= 2; i++, reward++)
    {
        if (value <= reward->maxValue && *ItemInventory_GetCountPtr(Item_GetTypeByGlobalId(reward->item), Item_GetIndexByGlobalId(reward->item)) == 0)
        {
            item = reward->item;
            goto found;
        }
    }
    item = 1;
found:
    entity->ext.bossRushResult.rewardItem = item;
    child = Pickup_CreateItem(entity->posX.part16.integer, (u16)entity->posY.part16.integer, 0xFF, item, 1);
    if (child != NULL)
        child->subState = 1;
    entity->scrollWithBg = 0;
    entity->hidden = 1;
    entity->posX.part16.integer = 0x80;
    entity->posY.part16.integer = 0x50;
    entity->drawFunc = BossRushResult_Draw;
}

/**
 * @brief 511F0 | Object 0x10 (save point?): when stateFlags bit 10 is set,
 * wait 128 frames with the player input cleared, then reload the current
 * save slot (SaveData_LoadSlotFromSram), call ItemInventory_AddOneByGlobalId(unk_4FC) and
 * SaveData_UpdateSlotSummary, store entity 26's unk_4F8 into bestBossRushTimeFrames when bestBossRushTimeFrames is 0 or
 * larger, and write the slot back (SaveData_SaveSlotToSram); finally set
 * subMode = 2, subModeStep = 0 (state 3).
 */
void BossRushResultUpdate(struct EwramData_EntityData *entity)
{
    struct GameState *data = &gEwramData->gameState;
    u8 state = entity->state;
    u32 value;

    switch (state)
    {
    case 0:
        if (gEwramData->gameState.stateFlags & 0x400)
        {
            entity->timer = 0x80;
            entity->state = 1;
            entity->hidden = 0;
        }
        break;
    case 1:
        SetPlayerInput(1, 0);
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        entity->state = 2;
        SaveData_LoadSlotFromSram(gEwramData->gameState.currentSaveSlot);
        ItemInventory_AddOneByGlobalId(entity->ext.bossRushResult.rewardItem);
        SaveData_UpdateSlotSummary(&gEwramData->gameState);
        {
            struct EwramData_EntityData *slot26 = &gEwramData->entityData[26];
            value = slot26->ext.bossRushTimer.time;
        }
        if (data->bestBossRushTimeFrames > value || data->bestBossRushTimeFrames == 0)
            data->bestBossRushTimeFrames = value;
        SaveData_SaveSlotToSram(gEwramData->gameState.currentSaveSlot);
        break;
    case 2:
        SetPlayerInput(1, 0);
        gEwramData->gameState.subMode = state;
        gEwramData->gameState.subModeStep = 0;
        entity->state = 3;
        break;
    }
}

/* Objects 0x11 (timer in entity slot 26) and 0x12 (decorations). */

/**
 * @brief 512C4 | Object 0x11 create: replace entity slot 26 by a counter
 * entity (update BossRushTimer_Update).
 */
void BossRushTimerStarterCreate(struct EwramData_EntityData *entity)
{
    EntityDelete(&gEwramData->entityData[26]);
    EntityCreateInRange(0x1A, 0x1A, BossRushTimer_Update)->roomActive = 1;
}

/**
 * @brief 512F8 | Object 0x11 update: nothing.
 */
void BossRushTimerStarterUpdate(struct EwramData_EntityData *entity)
{
}

/**
 * @brief Timer entity (slot 26): count unk_4F8 up to 0x34BBE (one hour at 60
 * frames per second) while bit 1 of stateFlags (no saving) is set.
 */
void BossRushTimer_Update(struct EwramData_EntityData *entity)
{
    entity->roomActive = 1;
    if ((gEwramData->gameState.stateFlags & 2) && entity->ext.bossRushTimer.time <= 0x34BBD)
        entity->ext.bossRushTimer.time++;
}

/**
 * @brief Update of object 0x12 variant 0 (sRoomDecorationVariants[0].updateFunc): idle
 * animation cycle sRoomDecorationVariant0AnimScript0, sRoomDecorationVariant0AnimScript1, sRoomDecorationVariant0AnimScript2 with
 * random waits.
 */
void RoomDecorationVariant0_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
            Anim_Start(entity, (struct AnimScript *)sRoomDecorationVariant0AnimScript0, 0, 0);
            entity->timer = (RandomNumberGenerator() & 0x7F) + 8;
            entity->state++;
        case 1:
            if (entity->timer != 0)
            {
                entity->timer--;
                break;
            }
            Anim_Start(entity, (struct AnimScript *)sRoomDecorationVariant0AnimScript1, 0, 0);
            entity->state++;
            break;
        case 2:
            if (entity->animFlags & 4)
            {
                entity->timer = (RandomNumberGenerator() & 0x7F) + 0x10;
                entity->state++;
            }
            break;
        case 3:
            if (entity->timer != 0)
            {
                entity->timer--;
                break;
            }
            Anim_Start(entity, (struct AnimScript *)sRoomDecorationVariant0AnimScript2, 0, 0);
            entity->state++;
            break;
        case 4:
            if (entity->animFlags & 4)
                entity->state = 0;
            break;
    }
    Anim_Update(entity);
}

/**
 * @brief 5140C | Object 0x12 create: load the graphics of variant spawnParam0,
 * start its animation, layer, and convert room coordinates (flag 2).
 */
void RoomDecorationCreate(struct EwramData_EntityData *entity)
{
    const struct RoomDecorationVariantDefinition *def = &sRoomDecorationVariants[entity->spawnParam0];

    if (!Entity_LoadGraphics(entity, def->graphics, def->palette, def->paletteIndex, def->spriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    if (def->updateFunc == NULL && !(def->flags & 4))
    {
        Anim_StartIndexed(entity, def->animationIndex, 0, 1);
        if (def->flags & 1)
            entity->objMode = 1;
    }
    entity->spriteLayer = def->spriteLayer;
    if (!(def->flags & 2))
    {
        entity->scrollWithBg = 1;
    }
    else
    {
        u16 *pos;
        struct BgLayerState *bg;

        entity->scrollWithBg = 0;
        pos = &entity->posX.part16.integer;
        bg = &gEwramData->bgInfo[1];
        *pos += bg->xPos.part16.integer;
        pos += 2;
        *pos += bg->yPos.part16.integer;
    }
}

/**
 * @brief 514D8 | Variant with spawnParam1 == 1 scrolls with background 3
 * instead of background 1; set drawFunc RoomDecorationVariant2_Draw on the first frame.
 */
void RoomDecorationVariant2_Update(struct EwramData_EntityData *entity)
{
    if (entity->state == 0)
    {
        if (entity->spawnParam1 == 1)
        {
            struct EwramData *ewram = gEwramData;
            struct BgLayerState *mainBg = &ewram->bgInfo[1];
            struct BgLayerState *farBg = &ewram->bgInfo[3];

            entity->posX.whole += mainBg->xPos.whole - farBg->xPos.whole;
        }
        entity->drawFunc = RoomDecorationVariant2_Draw;
        entity->state++;
    }
    if (entity->spawnParam1 == 1)
    {
        struct BgLayerState *farBg = &gEwramData->bgInfo[3];

        entity->velX.whole = farBg->xVel.whole;
        entity->posX.whole += entity->velX.whole;
    }
}

/**
 * @brief 5153C | Draw frames 0..3 (0 and 1 only while not above the
 * screen), toggling the OBJ mode bit every frame.
 */
void RoomDecorationVariant2_Draw(struct EwramData_EntityData *entity)
{
    s32 i;

    entity->objMode = 1 - entity->objMode;
    for (i = 0; i <= 3; i++)
    {
        if ((s16)entity->posY.part16.integer >= -0x10 || i > 1)
        {
            entity->animFrame = i;
            Sprite_Draw(entity);
        }
    }
}

/**
 * @brief 5158C | Object 0x12 update: (only near the screen for flag 8) run
 * the variant's update or the plain animation.
 */
void RoomDecorationUpdate(struct EwramData_EntityData *entity)
{
    const struct RoomDecorationVariantDefinition *def = &sRoomDecorationVariants[entity->spawnParam0];

    if ((def->flags & 8) && !Entity_IsInRect(entity, -0x80, -0x40, 0x1F0, 0x120))
        return;
    if (def->updateFunc != NULL)
        def->updateFunc(entity);
    else if (!(def->flags & 4))
        Anim_Update(entity);
}

/* WreckingBallArmView segment object: copies the angle of entity[hitFlashTimer], places itself
 * at half the (cos, sin) * 8 offset from its parent (unk_4FC) and keeps the
 * full offset for the next segment; rotated via ObjAffine_Set. */

struct WreckingBallArmView
{
    void *parent;
    s32 angle;
    s32 tipTargetAngle;
    s32 x;
    s32 y;
    u8 segmentIndex;
    u8 ownerSlot;
};
#define ARM(e) ((struct WreckingBallArmView *)&((struct EwramData_EntityData *)(e))->ext.raw.unk_4FC)

/**
 * @brief One of the ten affine segments of object 0x13, linked to the previous
 * part.
 */
void WreckingBallSegment_Update(struct EwramData_EntityData *entity)
{
    struct WreckingBallArmView *self = ARM(entity);
    struct WreckingBallArmView *p = self->parent;
    struct EwramData_EntityData *other = &gEwramData->entityData[self->ownerSlot];
    struct WreckingBallArmView *a = ARM(other);

    if (other->state == 0xFF)
        entity->state = 0xFF;
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sArenaTrapGfx, sUnk_08209FD8, 0, sArenaTrapSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        Anim_StartIndexed(entity, 1, 0, 0);
        Anim_Update(entity);
        entity->spriteLayer = 3;
        entity->state++;
    case 1:
        self->angle = a->angle;
        {
        struct WreckingBallArmView *a2 = ARM(entity), *p2 = ARM(p);
        a2->y = Sin(a2->angle + 0x4000) << 3;
        a2->x = Sin(a2->angle + 0x8000) << 3;
        entity->posX.whole = (a2->x >> 1) + p2->x;
        entity->posY.whole = a2->y / 2 + p2->y;
        a2->x += p2->x;
        a2->y += p2->y;
        }
        break;
    case 0xFF:
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        return;
    }
    ObjAffine_Set(self->angle, 0x10000, 0x10000, 0, 0, entity->affineIndex);
}

/* WreckingBallTip_Update, decompiled from the original assembly. The mode-0 branch
 * repeats the "+= 0x300" body instead of jumping to it: cross-jumping merges
 * the copies after reload, but their reloads shift the reload register
 * rotation as in the target. */

#define PL (gEwramData->playerState.player)
#define PX ((s16)PL->posX.part16.integer)
#define EX ((s16)entity->posX.part16.integer)
static inline void ArmPlace(struct EwramData_EntityData *entity, struct WreckingBallArmView *p)
{
    struct WreckingBallArmView *a2 = ARM(entity), *p2 = ARM(p);
    a2->y = Sin(a2->angle + 0x4000) << 5;
    a2->x = Sin(a2->angle + 0x8000) << 5;
    entity->posX.whole = (a2->x >> 1) + p2->x;
    entity->posY.whole = a2->y / 2 + p2->y;
    a2->x += p2->x;
    a2->y += p2->y;
}
/**
 * @brief Last part of object 0x13: has a hit box, checks player hits with a
 * hit effect and sound 0x69, queued for the player while near.
 */
void WreckingBallTip_Update(struct EwramData_EntityData *entity)
{
    struct WreckingBallArmView *self = ARM(entity);
    struct WreckingBallArmView *p = self->parent;
    struct EwramData_EntityData *other = &gEwramData->entityData[self->ownerSlot];
    struct WreckingBallArmView *a = ARM(other);
    struct EwramData_EntityData *player;
    struct HitResult res;
    s32 tx, ty, v, mode, r;

    if (other->state == 0xFF)
        entity->state = 0xFF;
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sArenaTrapGfx, sUnk_08209FD8, 0, sArenaTrapSpriteSheet))
        {
            EntityDelete(entity);
            break;
        }
        Anim_StartIndexed(entity, 0, 0, 0);
        Anim_Update(entity);
        entity->spriteLayer = 3;
        Hit_Enable(entity, 8, 0, 0);
        HitBox_SetBodyRaw(entity, 0x2020F4F0);
        HitBox_SetAttackRaw(entity, 0x2020F4F0);
        entity->hit.flags |= 2;
        entity->state++;
    case 1:
        r = 0x60;
        tx = Sin(a->tipTargetAngle + 0x8000) * r + a->x;
        ty = Sin(a->tipTargetAngle + 0x4000) * r + a->y;
        self->angle = a->angle;
        ArmPlace(entity, p);
        entity->velX.whole = tx - entity->posX.whole;
        entity->velY.whole = ty - entity->posY.whole;
        if (entity->velY.whole < 0)
            entity->velY.whole = 0;
        player = gEwramData->playerState.player;
        if (!Entity_IsInRect(entity, (s16)(player->posX.part16.integer - 0x50), (s16)(player->posY.part16.integer - 0x80), 0xA0, 0x100))
            break;
        Player_QueueEntity(entity, 3);
        Hit_CheckWithPlayer(&res, entity);
        if (!(res.flags & 2))
            break;
        v = other->velX.whole & 0xFFFF;
        other->velX.whole = v;
        mode = 0;
        if ((res.flags & 2) && (s16)gEwramData->playerState.player->posY.part16.integer < (s16)entity->posY.part16.integer)
            break;
        HitEffect_Spawn((s16)entity->posX.part16.integer + RandomSigned(0x10), (s16)entity->posY.part16.integer + RandomSigned(0x10), 0);
        PlaySong(0x69);
        if ((u32)(v - 0x4001) <= 0x7FFE)
            mode = 1;
        else if (other->accelX.whole != 0)
            mode = 2;
        if (mode == 0)
        {
            if (PX < EX)
                other->velX.whole = 0x8000;
            other->accelX.whole += 0x300;
        }
        else if ((mode == 1 && PX < EX && IsNotFlippedX(PL) == mode) || (mode == 2 && PX > EX && !IsNotFlippedX(PL)))
            other->accelX.whole += 0x300;
        else
            other->accelX.whole -= 0x60;
        break;
    case 0xFF:
        EntityDelete(entity);
        break;
    }
}

void WreckingBallCreate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *previous;
    s32 i;
    if (!Entity_LoadGraphics(entity, sArenaTrapGfx, sUnk_08209FD8, 0, sArenaTrapSpriteSheet)) { EntityDelete(entity); return; }
    entity->drawFunc = NULL;
    entity->hidden = 1;
    entity->scrollWithBg = 1;
    previous = entity;
    for (i = 0; i <= 10; i++)
    {
        u8 *data;
        struct EwramData_EntityData *child;
        if (i != 10) child = EntityCreateInRange(0x49, 0xCF, WreckingBallSegment_Update);
        else child = EntityCreateInRange(0x49, 0xCF, WreckingBallTip_Update);
        if (child == NULL) { entity->state = 0xFF; break; }
        if (i != 10)
        {
            u16 slot = ObjAffine_Alloc(1);
            if ((s16)slot < 0) { entity->state = 0xFF; break; }
            child->affineIndex = slot;
            child->affine = 1;
        }
        child->scrollWithBg = 1;
        /* General-purpose slots: previous part, zeroed value at +4,
         * part index at +0x14 and the owner's value at +0x15. */
        data = (u8 *)&child->ext.raw.unk_4FC;
        child->ext.raw.unk_4FC.unk_4FC_entity = previous;
        *(s32 *)(data + 4) = 0;
        data[0x14] = i;
        data[0x15] = ENTITY_INDEX(entity);
        previous = child;
    }

}

/* WreckingBallUpdate. */
void WreckingBallUpdate(struct EwramData_EntityData *entity)
{
    s32 *data = (s32 *)&entity->ext.raw.unk_4FC;
    if (entity->state == 0xFF)
    {
        if (entity->timer > 1) { EntityDelete(entity); return; }
        entity->timer++;
    }
    if (entity->accelX.whole > 0x2300) entity->accelX.whole = 0x2300;
    if (entity->spawnParam1 != 0) {
        u32 *base = gEwramData->gameState.gameFlagsTail;
        u32 *word = ({ s32 o_ = (entity->spawnParam1 >> 5) * 4; u32 *b_ = gEwramData->gameState.gameFlagsTail; (u32 *)((u8 *)b_ + o_); });
        u32 bit = 1 << (entity->spawnParam1 & 31);
        if (!(*word & bit)) {
            register s32 amplitude asm("r5"); /* FAKEMATCH: pinned register */
            s32 threshold;
            if (entity->spawnParam0 == 0) { amplitude = entity->accelX.whole; threshold = 0x2100; }
            else { amplitude = entity->accelX.whole; threshold = 0x1200; }
            if (amplitude > threshold && (u32)(entity->velX.whole - 0xAC01) <= 0x7FE) {
                entity->velX.whole -= 0x40;
                entity->accelX.whole = ({ register s32 t asm("r0") = amplitude - 0x100; t; }); /* FAKEMATCH: pinned register */
                *word |= bit;
            } } }
    if (entity->accelX.whole > 0)
    {
        entity->accelX.whole -= 4;
        entity->velX.whole = (entity->velX.whole + 0x200) & 0xFFFF;
    }
    else
    {
        entity->accelX.whole = 0;
        entity->velX.whole = 0;
    }
    data[1] = data[2];
    data[2] = (Sin(entity->velX.whole) * entity->accelX.whole) / 0x10000;
    data[3] = entity->posX.whole;
    { u32 tmp_ = entity->posY.whole; data[4] = tmp_; }
}
