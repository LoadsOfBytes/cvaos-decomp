/* Update of item/pickup objects (Pickup_CreateMana large, Pickup_CreateGold): fall with
 * gravity until the ground (Collision_GetPushUp, probe +8 px for unk_51A <= 1),
 * or bob on a sine for unk_51A > 4; then wait 300 frames, blink 64 and
 * disappear; states 0x63-0x65 flash the palette. */
#include "code_08009A0.h"
#include "code_08001194.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "entity_hit.h"

static inline s32 RoomX(s32 pos) { struct BgLayerState *bg = &gEwramData->bgInfo[1]; return pos + (s16)bg->xPos.part16.integer; }
static inline s32 RoomY(s32 pos) { struct BgLayerState *bg = &gEwramData->bgInfo[1]; return pos + (s16)bg->yPos.part16.integer; }

/**
 * @brief Check player contact, apply type-dependent falling or bobbing motion, countdown expiration or palette-flash states, and optional animation.
 */
void Pickup_Update(struct EwramData_EntityData *entity)
{
    struct HitResult res;
    s32 dy;

    Hit_CheckWithPlayer(&res, (u8 *)entity);
    if (entity->unk_518.part8.enemyId <= 4)
    {
        s32 x, y, h;
        entity->posY.whole += entity->velY.whole;
        x = RoomX((s16)entity->posX.part16.integer);
        y = RoomY((s16)entity->posY.part16.integer);
        h = entity->accelY.part16.integer;
        dy = (s16)Collision_GetPushUp(x, entity->unk_518.part8.enemyId <= 1 ? y + 8 + h : y + h);
        entity->velY.whole += 0x2000;
        if ((u32)entity->velY.whole > 0x40000)
            entity->velY.whole = 0x40000;
        if (entity->unk_4FC.part16.unk_4FE[0] != 0)
            entity->unk_4FC.part16.unk_4FE[0]--;
        if (dy != 0)
        {
            entity->posY.part16.integer += dy;
            entity->velY.whole = 0;
            if (entity->state == 0)
            {
                entity->unk_4FC.part16.unk_4FC = 300;
                entity->state++;
            }
        }
    }
    else
    {
        s32 v = Sin((u8)entity->unk_4F8.part8.unk_4F8 << 9) / 4;
        entity->velY.whole = v;
        entity->unk_4F8.whole++;
        entity->posY.whole += v;
    }
    switch (entity->state)
    {
    case 1:
        if (--entity->unk_4FC.part16.unk_4FC == 0)
        {
            entity->timer = 64;
            entity->state++;
        }
        break;
    case 2:
        entity->timer--;
        entity->drawSkip = 1;
        if (entity->timer == 0)
            entity->deleteRequested = 1;
        break;
    case 0x63:
        entity->unk_510.part8.basePalette = entity->palette;
        entity->state = 0x64;
        entity->timer = 0x80;
        break;
    case 0x64:
        if (entity->timer != 0)
            entity->timer--;
        else
        {
            entity->palette = 4;
            entity->state = 0x65;
            entity->timer = 4;
        }
        break;
    case 0x65:
        if (entity->timer != 0)
            entity->timer--;
        else
        {
            entity->palette = entity->unk_510.part8.basePalette;
            entity->state = 0x64;
            entity->timer = 0x80;
        }
        break;
    }
    if (entity->stage != 0)
        Anim_Update(entity);
}
