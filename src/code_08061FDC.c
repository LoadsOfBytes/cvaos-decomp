/* Event object helpers 0x08061FDC..0x080620E7. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, u8 variant, void *sheet);
extern void Interaction_End(struct EwramData_EntityData *entity, s32 deleteEntity);
extern void Sound_Play(s32 song);
extern void Music_StartOverride(s32 song);

/* Graphics of the event object. */
extern u8 sEventScene19Gfx[];
extern u8 sJuliusEntityPalette[];
extern u8 sJuliusSpriteSheet[];

/**
 * @brief 61FDC | Load the graphics (layer 0x11); delete the entity and
 * return 0 when that fails.
 */
s32 EventScene19_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 result;
    u8 *palette = sJuliusEntityPalette;
    u8 *sheet = sJuliusSpriteSheet;

    if (Entity_LoadGraphics(entity, sEventScene19Gfx, palette, 0, sheet))
    {
        entity->spriteLayer = 0x11;
        result = 1;
    }
    else
    {
        EntityDelete(entity);
        result = 0;
    }
    return result;
}

/**
 * @brief 62020 | Release the graphics slot of sEventScene19Gfx and continue
 * with Interaction_End(1).
 */
void EventScene19_Finish(struct EwramData_EntityData *entity)
{
    ObjGfx_Unload(ObjGfx_Find(sEventScene19Gfx));
    Interaction_End(entity, 1);
}

/**
 * @brief Init of event scene variant 20 (Object 0x20 unk_514 = 20,
 * sEventSceneCreateFuncs[20]). Set stateFlags bit 0, BgPalette_StopColorFade and
 * ObjPalette_StopColorFade.
 */
void EventScene20_Init(void)
{
    gEwramData->gameState.stateFlags |= 1;
    BgPalette_StopColorFade();
    ObjPalette_StopColorFade();
}

/**
 * @brief Init of event scene variant 21 (Object 0x20 unk_514 = 21,
 * sEventSceneCreateFuncs[21]). End the entity (0x53D bit 3).
 */
void EventScene21_Init(struct EwramData_EntityData *entity)
{
    entity->deleteRequested = 1;
}

/**
 * @brief Update of event scene variant 21 (sEventSceneUpdateFuncs[21]); returns the
 * player input mask. Sound_Play(0x1040), Music_StartOverride(0x25), then
 * continue with Interaction_End(1).
 */
s32 EventScene21_Update(struct EwramData_EntityData *entity)
{
    Sound_Play(0x1040);
    Music_StartOverride(0x25);
    Interaction_End(entity, 1);
    return 0;
}

/**
 * @brief 62098 | Event wait step: first time set textWindowPause and start a
 * duration frame timer; returns 1 while waiting, 0 once done (stage 2).
 */
s32 EventScene22_Wait(struct EwramData_EntityData *entity, s32 duration)
{
    struct EwramData *ewram = gEwramData;
    s32 waiting = 1;

    if (entity->stage == 0)
    {
        ewram->gameState.textWindowPause = waiting;
        entity->timer = duration;
        entity->stage++;
    }
    else if (entity->stage == 1)
    {
        if ((u8)--entity->timer == 0xFF)
            entity->stage++;
    }
    else
    {
        waiting = 0;
    }
    return waiting;
}
