/*
 * Room objects 0x19-0x1C and the save point prompt (HBlank fade, markers).
 */
#include "objects_19_1c.h"
#include "core.h"
#include "graphics.h"
#include "entity_util.h"
#include "structs/ewram.h"
#include "sound.h"
#include "player_core.h"
#include "bg_and_collision.h"
#include "gameplay.h"
#include "save_point_entity.h"
#include "bg_effects.h"
#include "structs/entity_render_flags.h"
#include "map_flags.h"
#include "save_data.h"
#include "enemy_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "palette.h"
#include "save_point.h"
#include "text12.h"
#include "data/data_0E5364.h"
#include "data/data_118B48.h"
#include "gba/memory.h"
#include "entity_hit.h"

/* Object 19: move along a timed path of steps, then despawn. */
struct Object19Step
{
    s32 time;
    s32 velX;
    s32 velY;
};

extern const struct Object19Step sObject19Steps[4];
extern s32 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern s32 Bg_SetControlIfLevelAllows(s32 level, s32 reg, s32 value);

static inline s32 Object19NextStep(struct EwramData_EntityData *entity, s32 step)
{
    const struct Object19Step *next;
    s32 time;

    entity->state = step;
    next = &sObject19Steps[step];
    entity->velX.whole = next->velX;
    entity->velY.whole = next->velY;
    if (entity->spawnParam1 != 0)
        entity->velX.whole = -entity->velX.whole;
    time = sObject19Steps[step].time;
    if (time == 0)
        time = entity->spawnParam0 << 16;
    return time;
}

#define OBJ19_STEP(entity, step)                                           \
    {                                                                      \
        s32 time_;                                                         \
        entity->posX.whole += entity->velX.whole;                          \
        entity->posY.whole += entity->velY.whole;                          \
        if ((s32)entity->ext.object19.stepTime > 0)                      \
            { time_ = entity->ext.object19.stepTime - 0x10000; asm(""); } /* FAKEMATCH: empty asm keeps jump2 from cross-jumping the copies */ \
        else                                                               \
            time_ = Object19NextStep(entity, step);                        \
        entity->ext.object19.stepTime = time_;                           \
    }

void Object19Update(struct EwramData_EntityData *entity)
{
    struct HitResult result;

    if (Entity_IsInRect(entity, -0x10, 0, 0x110, 0xB0) == 0)
    {
        entity->hidden = 1;
        return;
    }
    entity->hidden = 0;
    entity->ext.object19.angle += entity->ext.object19.spin;
    switch (entity->state)
    {
    case 0:
        OBJ19_STEP(entity, 1);
        break;
    case 1:
        OBJ19_STEP(entity, 2);
        break;
    case 2:
        OBJ19_STEP(entity, 3);
        break;
    case 3:
        OBJ19_STEP(entity, 0);
        break;
    }
    Hit_CheckWithPlayer(&result, entity);
    ObjAffine_Set((s16)entity->ext.object19.angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
}

/* Object 0x19 helpers 0x08053EC4..0x08053F33. */

/**
 * @brief Hit callback of object 0x19: sound 0x69 and a hit effect at a random
 * spot (0..15 px) from the entity.
 */
void Object19_OnHit(struct EwramData_EntityData *entity)
{
    s32 x;

    PlaySong(0x69);
    x = (s16)entity->posX.part16.integer + RandomSigned(0x10);
    HitEffect_Spawn(x, (s16)entity->posY.part16.integer + RandomSigned(0x10), 0);
}

/**
 * @brief Draw frame 0 without and frame 1 with bit 4 of 0x53D.
 */
void Object19_Draw(struct EwramData_EntityData *entity)
{
    entity->affine = 0;
    entity->animFrame = 0;
    Sprite_Draw(entity);
    entity->affine = 1;
    entity->animFrame = 1;
    Sprite_Draw(entity);
}

void Object1BCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sObject1BGfx, sUnk_08209FD8, 1, sObject1BSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 2, 0, 0);
    Anim_Update(entity);
    entity->spriteLayer = 3;
    Hit_Enable(entity, 11, NULL, 0);
    HitBox_SetBodyRaw(entity, 0x1010F0F8);
    entity->hit.flags |= 2;
    entity->hit.flags |= 4;
    gEwramData->gameState.conveyorSpeed = -0x14000;
}

void Object1BUpdate(struct EwramData_EntityData *entity)
{
    s32 index;
    u16 color;
    u16 *colors;
    Player_QueueEntity(entity, 0);
    switch (entity->state)
    {
    case 0:
        if (gEwramData->playerState.standingPlatform == entity)
        {
            entity->subState ^= 1;
            HitBox_SetBodyRaw(entity, 0x0C10F4F8);
            Anim_StartIndexed(entity, entity->subState + 2, 0, 0);
            PlaySong(0x133);
            entity->state = 1;
        }
        break;
    case 1:
        if (gEwramData->gameState.conveyorSpeed < 0x14000) gEwramData->gameState.conveyorSpeed += 0x1000;
        break;
    }
    entity->ext.object1B.scroll += gEwramData->gameState.conveyorSpeed;
    colors = sObject1BColors;
    index = ((entity->ext.object1B.scroll / 8) >> 16) % 3;
    if (index < 0) index = -index;
    color = colors[index];
    if ((u8)entity->ext.object1B.color != color)
    {
        entity->ext.object1B.color = color;
        BgPalette_LoadRange(sObject1BBgPalette, color, 1, 8);
    }
    Anim_Update(entity);
}

/* Object 0x1A has no create or update behavior. */
void Object1ACreate(struct EwramData_EntityData *entity)
{
}

void Object1AUpdate(struct EwramData_EntityData *entity)
{
}

/* Tile rows and text for the menu at 0x080540A8.
 * The first row uses its own counter; later rows reuse the pointer and counter
 * to preserve agbcc register allocation. The final copies retain size 0x18.
 */

/**
 * @brief Unreferenced variant of SavePoint_ShowSavePrompt drawing at row 0x15
 * with a highlighted second text.
 */
void SavePoint_ShowSavePromptLow(void)
{
    s32 y = 0x15;
    s32 i;
    s16 *buf;

    BgPalette_LoadRange(sSavePromptBgPalette, 0, 2, 0xD);
    Text_SetBackgroundColor(3);
    Text12_SetRegion(0, 0, 0x29, 2);
    Text12_ClearRegion();
    Text12_SetCursor(0, 1);
    Text12_Print(Text_GetLocalizedEntry(0x34A), 0);
    Text_SetBackgroundColor(0xD);
    Text12_SetCursor(0x10, 1);
    Text12_Print(Text_GetLocalizedEntry(0x34B), 0);
    Text_SetBackgroundColor(3);
    Text12_SetCursor(0x18, 1);
    Text12_Print(Text_GetLocalizedEntry(0x34D), 0);
    Text_SetBackgroundColor(3);
    {
        s16 *first = (s16 *)gEwramData->sharedScratchBuffer;
        s32 j;
        for (j = 0; j <= 4; j++)
            *first++ = (j + 0xAC) | (s16)0xE200;
        DmaQueue_DirectCopy(10, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(VRAM_BASE + 0xE00E + ((y + 2) << 6)));
    }
    {
        buf = (s16 *)gEwramData->sharedScratchBuffer;
        for (i = 0; i <= 4; i++)
            *buf++ = (i + 0xCC) | (s16)0xE200;
        DmaQueue_DirectCopy(10, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(VRAM_BASE + 0xE00E + ((y + 3) << 6)));
    }
    {
        buf = (s16 *)gEwramData->sharedScratchBuffer;
        for (i = 0; i <= 4; i++)
            *buf++ = (i + 0xB2) | (s16)0xE200;
        DmaQueue_DirectCopy(10, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(VRAM_BASE + 0xE028 + ((y + 2) << 6)));
    }
    {
        buf = (s16 *)gEwramData->sharedScratchBuffer;
        for (i = 0; i <= 4; i++)
            *buf++ = (i + 0xD2) | (s16)0xE200;
        DmaQueue_DirectCopy(10, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(VRAM_BASE + 0xE028 + ((y + 3) << 6)));
    }
    {
        buf = (s16 *)gEwramData->sharedScratchBuffer;
        for (i = 0; i <= 23; i++)
            *buf++ = (i + 0xA0) | (s16)0xE200;
        DmaQueue_DirectCopy(24, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(VRAM_BASE + 0xE014 + (y << 6)));
    }
    {
        buf = (s16 *)gEwramData->sharedScratchBuffer;
        for (i = 0; i <= 23; i++)
            *buf++ = (i + 0xC0) | (s16)0xE200;
        DmaQueue_DirectCopy(24, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(VRAM_BASE + 0xE014 + ((y + 1) << 6)));
    }
}

/* (Matched: the last DMA base is pinned to r0 and added as "b_ - -0x14" so combine keeps the add.)
 * Tile rows and text for the menu at 0x08054280.
 * The first row uses its own counter; later rows reuse the pointer and counter
 * to preserve agbcc register allocation. The final copies retain size 0x18. */

/**
 * @brief Print the save prompt texts 0x34A/0x34B/0x34D on BG0 and draw the
 * prompt frame tiles (row 5).
 */
void SavePoint_ShowSavePrompt(void)
{
    s32 y = 5;
    u32 base = 0x0600E000;
    s32 i;
    s16 *buf;

    BgPalette_LoadRange(sSavePromptBgPalette, 0, 2, 0xD);
    Text_SetBackgroundColor(0);
    Text12_SetRegion(0, 0, 0x29, 2);
    Text12_ClearRegion();
    Text12_SetCursor(0, 1);
    Text12_Print(Text_GetLocalizedEntry(0x34A), 0);
    Text12_SetCursor(0x10, 1);
    Text12_Print(Text_GetLocalizedEntry(0x34B), 0);
    Text12_SetCursor(0x18, 1);
    Text12_Print(Text_GetLocalizedEntry(0x34D), 0);
    {
        buf = (s16 *)gEwramData->sharedScratchBuffer;
        for (i = 0; i <= 4; i++)
            *buf++ = (i + 0xAC) | (s16)0xE200;
        DmaQueue_DirectCopy(10, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(VRAM_BASE + 0xE010 + ((y + 3) << 6)));
    }
    {
        buf = (s16 *)gEwramData->sharedScratchBuffer;
        for (i = 0; i <= 4; i++)
            *buf++ = (i + 0xCC) | (s16)0xE200;
        DmaQueue_DirectCopy(10, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(VRAM_BASE + 0xE010 + ((y + 4) << 6)));
    }
    {
        buf = (s16 *)gEwramData->sharedScratchBuffer;
        for (i = 0; i <= 4; i++)
            *buf++ = (i + 0xB2) | (s16)0xE200;
        DmaQueue_DirectCopy(10, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(VRAM_BASE + 0xE028 + ((y + 3) << 6)));
    }
    {
        buf = (s16 *)gEwramData->sharedScratchBuffer;
        for (i = 0; i <= 4; i++)
            *buf++ = (i + 0xD2) | (s16)0xE200;
        DmaQueue_DirectCopy(10, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(VRAM_BASE + 0xE028 + ((y + 4) << 6)));
    }
    {
        buf = (s16 *)gEwramData->sharedScratchBuffer;
        for (i = 0; i <= 23; i++)
            *buf++ = (i + 0xA0) | (s16)0xE200;
        DmaQueue_DirectCopy(24, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(VRAM_BASE + 0xE014 + (y << 6)));
    }
    {
        buf = (s16 *)gEwramData->sharedScratchBuffer;
        for (i = 0; i <= 23; i++)
            *buf++ = (i + 0xC0) | (s16)0xE200;
        DmaQueue_DirectCopy(24, (u32 *)gEwramData->sharedScratchBuffer,
                           (u32 *)(((y + 1) << 6) + ({ register u32 b_ asm("r0") = base; b_ - -0x14; }))); /* FAKEMATCH: pinned register */
    }
}

/* Unreferenced H-blank table fill between the save prompt functions (matches with an s16 cast). */
/**
 * @brief Unreferenced: fill the back H-blank buffer: lines 0..0x23 with 6,
 * 0x24..0x2F with 0x7C + offset, 0x30..0x4F - offset with 0, the rest up
 * to 0x4F with 0x6C + offset, then a 0 terminator, and request the start.
 */
void SavePoint_FillPromptHBlankBuffer(s32 offset)
{
    s32 base = 0xAC;
    s16 *dst = gEwramData->hBlankEffect.hBlankBuffer[1 - gEwramData->hBlankEffect.currentBuffer];
    s32 i;
    s32 gapEnd;

    for (i = 0; i <= 0x23; i++)
        *dst++ = 6;
    for (i = 0; i <= 0xB; i++)
        *dst++ = (s16)(base - 0x30) + offset;
    i = 0x30;
    gapEnd = 0x50 - offset;
    for (; i < gapEnd; i++)
        *dst++ = 0;
    for (; i <= 0x4F; i++)
        *dst++ = (s16)(base - 0x40) + offset;
    *dst = 0;
    gEwramData->hBlankEffect.requestStart = 1;
}

/* SavePoint_DrawPromptMarkers (0x080544E8..0x08054573). */
/**
 * @brief Draw the prompt sprite parts at fixed screen positions; frames
 * 0x16/0x17 and 0x19/0x18 follow the yes/no choice unk_4FA of the save
 * point in slot unk_4F5.
 */
void SavePoint_DrawPromptMarkers(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *prompt = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];

    if (prompt != NULL)
    {
        entity->posX.part16.integer = 0x48;
        entity->posY.part16.integer = 0x26;
        entity->animFrame = 0x15;
        Sprite_Draw(entity);
        entity->posX.part16.integer = 0x39;
        entity->posY.part16.integer = 0x3E;
        if (prompt->ext.savePoint.yesSelected)
            entity->animFrame = 0x16;
        else
            entity->animFrame = 0x17;
        Sprite_Draw(entity);
        entity->posX.part16.integer = 0x99;
        if (prompt->ext.savePoint.yesSelected)
            entity->animFrame = 0x19;
        else
            entity->animFrame = 0x18;
        Sprite_Draw(entity);
    }
}

/* Object 0x1C palette transition, returning one when the transition is done. */
/**
 * @brief Unreferenced fade step next to the save point code: fade the sprite in
 * (state 0, loading save point palette row 5) or out (state 1, restoring
 * the shared row) in 0x2000 steps of unk_4F8; return 1 when finished.
 */
s32 SavePoint_StepPromptFade(struct EwramData_EntityData *entity)
{
    s32 done = 0;
    switch (entity->state)
    {
    case 0:
        switch (entity->subState)
        {
        case 0:
            entity->hidden = 0;
            entity->ext.savePointPrompt.fade = 0;
            ObjPalette_LoadRange(sSaveWarpRoomPalette, 5, 1, 14);
            goto advance;
        case 1:
            entity->ext.savePointPrompt.fade += 0x2000;
            if ((s32)entity->ext.savePointPrompt.fade <= 0xFFFF) goto end;
            goto full;
        case 2:
            done = 1;
            break;
        }
        break;
    case 1:
        switch (entity->subState)
        {
        case 0:
            entity->hidden = 0;
        full:
            entity->ext.savePointPrompt.fade = 0x10000;
        advance:
            entity->subState++;
            break;
        case 1:
            entity->ext.savePointPrompt.fade -= 0x2000;
            if ((s32)entity->ext.savePointPrompt.fade <= 0)
            {
                entity->ext.savePointPrompt.fade = 0;
                entity->subState++;
                ObjPalette_LoadRange(sSavePointPalette, 14, 1, 14);
            }
            break;
        case 2:
            entity->hidden = 1;
            done = 1;
            break;
        }
        break;
    }
end:
    return done;
}

/**
 * @brief 5464C | Object 0x1C init: Hud_CloseMessageWindow, Text_SetTileBuffer(VRAM
 * 0x06005000), graphics sSaveWarpRoomGfx (ObjGfx_Find or newly loaded,
 * deleted when neither works) in unk_4FC, palette sSaveWarpRoomPalette to slot 10
 * (unk_4FE), affine slot in unk_4FB with scale unk_500, screen position
 * (0x78, 0x50), layer 0x11, Entity_SnapToGround / SavePoint_SpawnAnimParts; plus a helper
 * entity (layer 3, hidden, drawn by SavePoint_DrawPromptMarkers with sprite
 * sSaveWarpRoomSpriteSheet) whose unk_4F4 byte is kept in byte 1 of hp.
 */
void Object1CCreate(struct EwramData_EntityData *entity)
{
    u8 *gfx = sSaveWarpRoomGfx;
    u8 *pal = sSaveWarpRoomPalette;
    s32 graphics;
    struct EwramData_EntityData *child;

    Hud_CloseMessageWindow();
    Text_SetTileBuffer((void *)(VRAM_BASE + 0x5000));
    graphics = ObjGfx_Find(gfx);
    if (graphics < 0 && (graphics = ObjGfx_LoadResource(gfx, 0)) < 0)
    {
        EntityDelete(entity);
        return;
    }
    entity->ext.savePoint.graphics = graphics;
    ObjPalette_LoadRange(pal, 0, 4, 10);
    entity->ext.savePoint.frameBase = 10;
    entity->ext.savePoint.affine = ObjAffine_Alloc(1);
    entity->affineIndex = entity->ext.savePoint.affine;
    ObjAffine_Set(0, entity->ext.savePoint.scale, entity->ext.savePoint.scale, 0, 0, entity->ext.savePoint.affine);
    entity->posX.part16.integer = 0x78;
    entity->posY.part16.integer = 0x50;
    entity->spriteLayer = 0x11;
    Entity_SnapToGround(entity);
    SavePoint_SpawnAnimParts(entity);
    child = EntityCreateInRange(0x1B, 0x48, EntityUpdateNothing);
    if (child != NULL)
    {
        child->hidden = 1;
        child->spriteLayer = 3;
        SAVE_POINT_MARKERS_INDEX(entity) = ENTITY_INDEX(child);
        ENTITY_PARENT_INDEX(child) = ENTITY_INDEX(entity);
        child->drawFunc = SavePoint_DrawPromptMarkers;
        child->updateFunc = EntityUpdateNothing;
        Sprite_Attach(child, sSaveWarpRoomSpriteSheet, entity->ext.savePoint.graphics, 0xE);
    }
}

/* Object1CUpdate, decompiled from the original assembly (save point / save
 * room object 0x1C). Bg_SetControlIfLevelAllows is declared here with an s32 third
 * parameter: the original caller passes 0x10 - x without narrowing it to u16.
 * The state is read into a local before &gEwramData is copied into its own
 * register. */

#define OWNER(e) (&(*ew)->entityData[ENTITY_BYTE_519((e))])

void Object1CUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData **ew;
    u8 state;
    u16 held = gEwramData->inputData.heldInput;
    u16 pressed = gEwramData->inputData.newInput;
    state = entity->state;
    ew = &gEwramData;

    switch (state)
    {
    case 0:
    {
        u8 sub = entity->subState;
        if (sub == 0)
        {
            if ((pressed & 0x40) && Entity_IsInRect((*ew)->playerState.player, 0x68, 0x80, 0x20, 0x37))
            {
                entity->subState++;
                entity->timer = sub;
                break;
            }
            gEwramData->gameState.stateFlags &= ~1;
            break;
        }
        {
            s32 r = Player_IsBusy();
            if (r == 0)
            {
                PlaySong(0xF3);
                BgPalette_StopColorFade();
                ObjPalette_StopColorFade();
                RedSoul_CancelAll();
                gEwramData->gameState.stateFlags |= 1;
                SavePoint_ShowSavePrompt();
                Player_RestoreHPAndMP();
                gEwramData->startPauseEnabled = 0;
                entity->ext.savePoint.yesSelected = 1;
                entity->subState = r;
                goto next;
            }
            if (entity->timer == 3)
                entity->subState = 0;
        }
        break;
    }
    case 1:
    {
        struct EwramData_EntityData *owner = OWNER(entity);
        ObjPalette_LoadRange(sSaveWarpRoomPalette, 4, 1, 0xE);
        owner->hidden = 0;
        goto next;
    }
    case 2:
    {
        s32 ab = pressed & 3;
        if (ab)
        {
            if ((held & 1) && entity->ext.savePoint.yesSelected)
            {
                struct EwramData *base = *ew;
                s32 res;
                SaveData_StorePlayerPosition(&base->gameState);
                SaveData_UpdateSlotSummary(&base->gameState);
                res = SaveData_SaveSlotToSram(base->gameState.currentSaveSlot);
                if (res == 0)
                {
                    struct EwramData_EntityData *owner;
                    Text_SetDefaultColors();
                    entity->timer = res;
                    entity->state = 0x14;
                    entity->ext.savePoint.flags |= 2;
                    PlaySong(0xF3);
                    owner = OWNER(entity);
                    ObjPalette_LoadRange(sSavePointPalette, 5, 1, 0xE);
                    owner->hidden = 1;
                    Text12_ClearRegion();
                    break;
                }
                else
                {
                    struct EwramData_EntityData *owner;
                    PlaySong(0xEF);
                    owner = OWNER(entity);
                    ObjPalette_LoadRange(sSavePointPalette, 5, 1, 0xE);
                    owner->hidden = 1;
                    Text12_ClearRegion();
                    BgPalette_LoadRange(sSavePromptBgPalette, 0, 2, 0xD);
                    Text_SetBackgroundColor(2);
                    goto reset;
                }
            }
            PlaySong(0xF0);
            goto next;
        }
        if ((held & 0x10) && entity->ext.savePoint.yesSelected)
        {
            PlaySong(0xF1);
            entity->ext.savePoint.yesSelected = ab;
            break;
        }
        if (!(held & 0x20))
            break;
        if (entity->ext.savePoint.yesSelected)
            break;
        PlaySong(0xF1);
        entity->ext.savePoint.yesSelected = 1;
        break;
    }
    case 3:
    {
        struct EwramData_EntityData *owner = OWNER(entity);
        ObjPalette_LoadRange(sSavePointPalette, 5, 1, 0xE);
        owner->hidden = 1;
        Text12_ClearRegion();
    }
    reset:
        (*ew)->startPauseEnabled = 1;
        entity->timer = 0;
        entity->state = 0;
        break;
    case 11:
    {
        if (SAVE_POINT_LIGHTS_IN_DONE(entity) == 0)
        {
            if (SAVE_POINT_LIGHTS_OUT_DONE(entity) == 0)
            {
                entity->ext.savePoint.scale += 0x800;
                if (entity->ext.savePoint.scale > 0xFFFF)
                    entity->ext.savePoint.scale = 0x10000;
                break;
            }
            entity->ext.savePoint.scale -= 0xCC;
            if (entity->ext.savePoint.scale <= 0)
                entity->ext.savePoint.scale = 0;
            break;
        }
        Bg_SetControlIfLevelAllows(3, 0x50, 0x3FBF);
        Bg_SetControlIfLevelAllows(3, 0x54, 0);
        goto clear;
    }
    case 12:
        Bg_SetControlIfLevelAllows(3, 0x54, entity->timer >> 2);
        if (entity->timer > 0x3F)
        {
            entity->hidden = 1;
            entity->timer = 0;
            entity->state++;
        }
        break;
    clear:
        entity->timer = 0;
    next:
        entity->state++;
        break;
    case 13:
        Bg_SetControlIfLevelAllows(3, 0x54, 0x10 - entity->timer);
        if (entity->timer <= 0xF)
            break;
        gEwramData->gameState.stateFlags &= ~1;
        goto done;
    case 10:
    case 20:
        if (SAVE_POINT_SAVE_DONE(entity) != 1)
            break;
        SavePoint_SpawnOrbitLights(entity);
        BgPalette_LoadRange(sUnk_08119898 + 0x378, 0xC, 2, 2);
        entity->state++;
        PlaySong(0xF6);
        break;
    case 21:
    {
        if (SAVE_POINT_LIGHTS_IN_DONE(entity) == 0)
        {
            if (SAVE_POINT_LIGHTS_OUT_DONE(entity) == 0)
            {
                entity->ext.savePoint.scale += 0x800;
                if (entity->ext.savePoint.scale > 0xFFFF)
                    entity->ext.savePoint.scale = 0x10000;
                if (entity->subState == 0)
                {
                    entity->subState = 1;
                    entity->timer = 0;
                }
            }
            else
            {
                entity->ext.savePoint.scale -= 0xCC;
                if (entity->ext.savePoint.scale <= 0)
                    entity->ext.savePoint.scale = 0;
            }
        }
        switch (entity->subState)
        {
        case 0:
            break;
        case 1:
            if (entity->timer == 0xE)
            {
                Bg_SetControlIfLevelAllows(3, 0x50, 0x3FBF);
                Bg_SetControlIfLevelAllows(3, 0x54, 0);
                entity->timer = 0;
                entity->subState++;
            }
        case 2:
            Bg_SetControlIfLevelAllows(3, 0x54, entity->timer >> 1);
            if (entity->timer > 0x1F)
            {
                Bg_SetControlIfLevelAllows(3, 0x54, 0x10);
                entity->subState++;
                entity->timer = 0;
            }
            break;
        case 3:
            Bg_SetControlIfLevelAllows(3, 0x54, 0x10 - (entity->timer >> 3));
            if (entity->timer > 0x3F)
                entity->subState++;
            break;
        case 4:
            if (SAVE_POINT_LIGHTS_IN_DONE(entity) == 1)
            {
                (*ew)->gameState.stateFlags &= ~1;
                Bg_SetControlIfLevelAllows(3, 0x54, 0);
                entity->timer = 0;
                entity->subState = 0;
            done:
                entity->state = 0x63;
            }
            break;
        }
        break;
    }
    case 99:
    {
        struct EwramData_EntityData *owner = OWNER(entity);
        ObjPalette_LoadRange(sSavePointPalette, 5, 1, 0xE);
        owner->hidden = 1;
        Text12_ClearRegion();
        entity->deleteRequested = 1;
        (*ew)->startPauseEnabled = 1;
        break;
    }
    }

    if (entity->state != 0)
        SetPlayerInput(1, 0x40);
    ObjAffine_Set(0, entity->ext.savePoint.scale, entity->ext.savePoint.scale, 0, 0, entity->ext.savePoint.affine);
    entity->timer++;
    if ((u8)(entity->timer % 3) == 0 && SAVE_POINT_SAVE_DONE(entity) != 1)
    {
        u8 *blink = &SAVE_POINT_BLINK(entity);
        *blink = 1 - *blink;
        if (*blink)
            BgPalette_LoadRange(sUnk_08119898 + 0x378, 0xA, 2, 2);
        else
            BgPalette_LoadRange(sUnk_08119898 + 0x378, 2, 2, 2);
    }
}
