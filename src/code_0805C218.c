/* EventScene31_SpawnBlockDebris..sub_0805C49B. */
#include "code_080D73B8.h"
#include "event_scene_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/* Debris piece of EventScene31_SpawnBlockDebris (8-byte records, 12 pieces). */
struct EventScene31DebrisDefinition {
    u8 frame;
    s8 dx;
    s8 dy;
    s8 vx;
    s8 vy;
    u8 initialTimer;
    u8 pad_6[2];
};

/* Palette animation step of EventScene31_StepPaletteAnim (4 steps). */
struct EventScene31PaletteStep {
    u8 *palette;
    u16 sourcePalette;
    u16 duration;
};

extern const struct EventScene31DebrisDefinition sEventScene31DebrisDefinitions[];
extern const struct EventScene31PaletteStep sEventScene31PaletteSteps[];
extern struct EwramData_EntityData *Entity_CreateFromObjectTable(s32 kind);
extern s32 Collision_GetGroundOffset(s32 x, s32 y);
s32 EventScene31Debris_Update(struct EwramData_EntityData *entity);

/* Enter state s of entity (state) with both sub-states cleared. */
#define SET_STATE(e, s)                     \
    {                                       \
        (e)->state = (s);                 \
        (e)->stage = 0;                   \
        (e)->subState = 0;                   \
    }

/**
 * @brief 5C218 | Spawn the 12 debris pieces of sEventScene31DebrisDefinitions around
 * (x, y): offset, velocity (y up), frame and gravity 0x2000, updated by
 * EventScene31Debris_Update.
 */
s32 EventScene31_SpawnBlockDebris(s16 x, s16 y)
{
    const struct EventScene31DebrisDefinition *info = sEventScene31DebrisDefinitions;
    s32 i;

    for (i = 0; i < 12; i++, info++)
    {
        struct EwramData_EntityData *entity = Entity_CreateFromObjectTable(11);

        if (entity != NULL)
        {
            entity->posX.part16.integer = x + info->dx;
            entity->posY.part16.integer = y + info->dy;
            entity->velX.whole = info->vx << 12;
            entity->velY.whole = -(info->vy << 12);
            entity->animFrame = info->frame;
            entity->timer = info->initialTimer;
            entity->accelY.whole = 0x2000;
            entity->updateFunc = (u32 *)EventScene31Debris_Update;
        }
    }
}

/**
 * @brief 5C284 | Update of a falling block: move with acceleration until
 * it hits the terrain (Collision_GetGroundOffset at its map position), then burst into
 * debris (EventScene31_SpawnBlockDebris, song 0xAA) and switch to frame 8.
 */
s32 EventScene31FallingBlock_Update(struct EwramData_EntityData *entity)
{
    u8 state = entity->state;

    if (state == 0)
    {
        struct BgLayerState *bg;
        s32 x = (s16)entity->posX.part16.integer;
        s32 hit;

        bg = &gEwramData->bgInfo[1];
        hit = Collision_GetGroundOffset(x + (s16)bg->xPos.part16.integer,
                           (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer);

        if (hit)
        {
            EventScene31_SpawnBlockDebris(entity->posX.part16.integer, entity->posY.part16.integer - 2);
            PlaySong(0xAA);
            entity->state++;
            entity->stage = state;
            entity->subState = state;
        }
        else
        {
            entity->posX.whole += entity->velX.whole;
            entity->posY.whole += entity->velY.whole;
            entity->velX.whole += entity->accelX.whole;
            entity->velY.whole += entity->accelY.whole;
            if (entity->velX.whole > 0)
                entity->velX.whole = hit;
        }
    }
    else if (entity->stage == 0)
    {
        entity->animFrame = 8;
        entity->stage++;
    }
}

/**
 * @brief 5C328 | Step the four-step palette animation sEventScene31PaletteSteps of
 * entity (EVENT31_PALETTE_TICK / _STEP): returns 0 after the last step,
 * else 1.
 */
s32 EventScene31_StepPaletteAnim(struct EwramData_EntityData *entity)
{
    const struct EventScene31PaletteStep *anim = &sEventScene31PaletteSteps[EVENT31_PALETTE_STEP(entity)];

    if (EVENT31_PALETTE_TICK(entity)++ > anim->duration)
    {
        EVENT31_PALETTE_TICK(entity) = 0;
        ObjPalette_LoadRange(anim->palette, anim->sourcePalette, 1, entity->unk_510.part8.basePalette);
        if (++EVENT31_PALETTE_STEP(entity) > 3)
        {
            EVENT31_PALETTE_STEP(entity) = 0;
            return 0;
        }
        anim = &sEventScene31PaletteSteps[EVENT31_PALETTE_STEP(entity)];
        ObjPalette_StartBlend(0, (u16 *)anim->palette, anim->sourcePalette, 0x10000, 0, anim->duration);
    }
    return 1;
}

/**
 * @brief 5C39C | Update of the block holder: state 1 waits for animDone,
 * state 2 drops the block (EventScene31FallingBlock_Update) and waits 0x20 frames, state 3
 * runs the palette animation (EventScene31_StepPaletteAnim), then state 0x80. States 0
 * and 1 run Anim_Update while idle.
 */
s32 EventScene31BlockHolder_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
            Anim_Update(entity);
            break;
        case 1:
            if (entity->stage == 0)
            {
                entity->animDone = 0;
                entity->stage++;
            }
            if (entity->animDone)
                SET_STATE(entity, entity->state + 1)
            else
                Anim_Update(entity);
            break;
        case 2:
            if (entity->stage == 0)
            {
                s32 x = (s16)(entity->posX.part16.integer - 0xB);
                s32 y = (s16)(entity->posY.part16.integer - 0x10);
                struct EwramData_EntityData *block = Entity_CreateFromObjectTable(11);

                if (block != NULL)
                {
                    block->posX.part16.integer = x;
                    block->posY.part16.integer = y;
                    block->velX.whole = -0xA000;
                    block->velY.whole = 0x2000;
                    block->accelX.whole = 0x100;
                    block->accelY.whole = 0x1000;
                    block->animFrame = 3;
                    block->updateFunc = (u32 *)EventScene31FallingBlock_Update;
                }
                entity->animFrame = 2;
                entity->stage++;
                entity->timer = 0x20;
            }
            if (--entity->timer == 0xFF)
                SET_STATE(entity, entity->state + 1)
            break;
        case 3:
            if (entity->stage == 0)
            {
                EVENT31_PALETTE_TICK(entity) = 0;
                EVENT31_PALETTE_STEP(entity) = 0;
                entity->stage++;
            }
            if (EventScene31_StepPaletteAnim(entity) == 0)
                SET_STATE(entity, 0x80)
            break;
    }
}
