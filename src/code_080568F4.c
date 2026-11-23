/* Object22_UpdateState. */
#include "code_080D73B8.h"
#include "room_object_entity.h"
#include "structs/ewram.h"

struct Seq { s8 level; s8 step; s8 hit; u8 flags; };
#define SEQ(e) ((struct Seq *)&(e)->unk_504)

extern void *memcpy(void *dst, const void *src, unsigned long size);
extern void EntityMoveX(struct EwramData_EntityData *);
extern void Object22_SpawnEffect(struct EwramData_EntityData *, s32, s32, u8);
extern void Object22_SpawnFallingPart(struct EwramData_EntityData *);
extern s32 Entity_MoveWithBoxCollision(struct EwramData_EntityData *, s32, s32, s32, s32, s32, s32);
extern s32 Entity_GetVisibilityPattern(s32);
extern u8 sWarpLightPaletteSteps[];
extern const s16 sObject22LevelThresholds[];
extern u8 *sObject22LevelPatterns[];


/**
 * @brief Main behaviour of object 0x22, called by Object22Update.
 */
void Object22_UpdateState(struct EwramData_EntityData *e)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    u8 durations[5];
    u8 *pat;

    memcpy(durations, sWarpLightPaletteSteps + 0x30, 5);
    switch (e->subState)
    {
    case 0:
        if (gEwramData->playerState.standingPlatform == e)
            e->posY.part16.integer = 0x9E - bg->yPos.part16.integer;
        else
            e->posY.part16.integer = 0x9D - bg->yPos.part16.integer;
        if (gEwramData->playerState.standingPlatform == e)
        {
            if (e->timer++ > 0x3B)
            {
                e->subState++;
                SEQ(e)->level = 1;
                e->timer = 0;
                e->velX.whole = 0x10000;
            }
        }
        else
            e->timer = 0;
        break;
    case 1:
        EntityMoveX(e);
        if (gEwramData->playerState.standingPlatform != e)
        {
            e->posY.part16.integer = 0x9D - bg->yPos.part16.integer;
            if (++OBJECT22_EMPTY_TIMER(e) > 0x3B)
            {
                e->stage = 0;
                e->subState = 4;
                e->timer = 0x14;
                break;
            }
        }
        else
        {
            e->posY.part16.integer = 0x9E - bg->yPos.part16.integer;
            OBJECT22_EMPTY_TIMER(e) = 0;
        }
        if ((s16)e->posX.part16.integer + (s16)bg->xPos.part16.integer >= sObject22LevelThresholds[SEQ(e)->level])
        {
            e->posX.part16.integer = sObject22LevelThresholds[SEQ(e)->level] - bg->xPos.part16.integer;
            e->velX.whole = 0;
            e->subState++;
            e->stage = 0;
            e->timer = 0;
        }
        break;
    case 2:
        { u8 **pt = sObject22LevelPatterns; s32 i = SEQ(e)->level - 1; pat = pt[i]; }
        if (gEwramData->playerState.standingPlatform != e)
        {
            e->posY.part16.integer = 0x9D - bg->yPos.part16.integer;
            if (++OBJECT22_EMPTY_TIMER(e) > 0x3B)
            {
                e->stage = 0;
                e->subState = 4;
                e->timer = 0x14;
                break;
            }
        }
        else
        {
            e->posY.part16.integer = 0x9E - bg->yPos.part16.integer;
            OBJECT22_EMPTY_TIMER(e) = 0;
        }
        switch (e->stage)
        {
        case 0:
            if (e->timer++ > 0x1D)
            {
                e->stage++;
                e->timer = 0;
            }
            break;
        case 1:
        {
            s32 dx, dy;
            u8 key = pat[SEQ(e)->step + 1];
            if (key == 0x40)
            {
                dx = 0;
                dy = 0x6B0000;
            }
            else if (key == 0x10)
            {
                dx = 0x60000;
                dy = 0x720000;
            }
            else if (key == 0x80)
            {
                dx = 0;
                dy = 0x790000;
            }
            else if (key == 0x20)
            {
                dx = -0x60000;
                dy = 0x720000;
            }
            { const s16 *tbl = sObject22LevelThresholds; dx += tbl[SEQ(e)->level] << 16; }
            dx -= bg->xPos.whole;
            dy -= bg->yPos.whole;
            Object22_SpawnEffect(e, dx, dy, durations[SEQ(e)->level - 1]);
            e->stage++;
            e->timer = 0;
            SEQ(e)->hit = 0;
            PlaySong(0x15B);
            break;
        }
        case 2:
            if (++e->timer <= durations[SEQ(e)->level - 1] && SEQ(e)->hit == 0)
            {
                u16 in = gEwramData->inputData.playerNewInput;
                if (pat[SEQ(e)->step + 1] & in)
                {
                    SEQ(e)->hit = 1;
                    PlaySong(0x15A);
                }
                else if (in & 0xF0)
                {
                    SEQ(e)->hit = 2;
                    e->timer = durations[SEQ(e)->level - 1];
                    PlaySong(0x15A);
                }
            }
            if (e->timer >= durations[SEQ(e)->level - 1] + 0x1E)
            {
                if (SEQ(e)->hit == 1)
                {
                    e->timer = 0;
                    SEQ(e)->step++;
                    if (SEQ(e)->step >= pat[0])
                    {
                        SEQ(e)->step = 0;
                        e->stage = 0;
                        e->subState++;
                    }
                    else
                        e->stage = 1;
                }
                else
                {
                    e->stage = 0;
                    e->subState = 4;
                    e->timer = 0x14;
                }
            }
            break;
        }
        break;
    case 3:
        if (gEwramData->playerState.standingPlatform == e)
            e->posY.part16.integer = 0x9E - bg->yPos.part16.integer;
        else
            e->posY.part16.integer = 0x9D - bg->yPos.part16.integer;
        if (e->timer++ << 24)
        {
            if (++SEQ(e)->level > 5)
            {
                e->subState = 10;
                e->timer = 0;
            }
            else
            {
                e->timer = 0;
                e->velX.whole = 0x10000;
                e->subState = 1;
            }
        }
        break;
    case 4:
        if (e->stage == 0)
        {
            e->stage++;
            e->velX.whole = 0;
            e->accelX.whole = 0;
            Object22_SpawnFallingPart(e);
            PlaySong(0x15C);
        }
        if ((s8)--e->timer <= 0)
        {
            SEQ(e)->flags |= 1;
            e->timer = 0x28;
            e->subState++;
        }
        break;
    case 5:
        Entity_MoveWithBoxCollision(e, 0x10000, -0x10, 0, 0x20, 0xC, 0x60000);
        e->hidden = Entity_GetVisibilityPattern(e->timer);
        if ((s8)--e->timer <= 0)
            e->deleteRequested = 1;
        break;
    case 10:
        gEwramData->gameState.gameFlagsTail[1] |= 0x8000;
        e->subState++;
        break;
    case 11:
        break;
    }
}
