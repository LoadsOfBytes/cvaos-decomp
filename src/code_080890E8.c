/* Ground activation, timed gait, turning and backward retreat. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include <string.h>
extern s8 sGorgonWalk[16], sGorgonTurn[12];
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s16);
extern s32 Target_IsInRect(s16, s16, s32, s32);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Tile_Classify(s16, s16);
#define GAIT(e) ((e)->unk_4FC.part8.unk_4FD)
#define FRAME(e) ((e)->unk_4FC.part8.unk_4FE)
#define CLOCK(e) ((e)->unk_4FC.part8.unk_4FF)
static inline s32 TileAhead(struct EwramData_EntityData *entity, struct BgLayerState *background, s32 offset)
{
    return Tile_Classify((s16)(background->xPos.part16.integer + entity->posX.part16.integer + offset),
                       (s16)(background->yPos.part16.integer + entity->posY.part16.integer));
}
/**
 * @brief Walk state machine (state 0): fall to the ground, wake when the target is
 * in the 180 x 120 box or the Gorgon is damaged, then walk with the gait
 * tables sGorgonWalk / sGorgonTurn and start the breath attack.
 */
void Gorgon_UpdateWalk(struct EwramData_EntityData *entity)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    s8 walk[16], turn[12];
    s32 phase;
    memcpy(walk, sGorgonWalk, sizeof(walk));
    memcpy(turn, sGorgonTurn, sizeof(turn));
    phase = entity->subState;
    switch (phase)
    {
    case 0:
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2000;
        if (!(Entity_MoveWithCollision(entity, 0x40000, -0x200000, 0x80000, 24) & 4)) break;
        if (Target_IsInRect((s16)(entity->posX.part16.integer - 90),
                         (s16)(entity->posY.part16.integer - 60), 180, 120) || Enemy_IsDamaged(entity))
        {
            entity->subState++;
            CLOCK(entity) = phase;
            GAIT(entity) = phase;
            entity->velX.whole = phase;
        }
        break;
    case 1:
    {
        s32 offset;
        entity->unk_4FC.part8.unk_4FC |= 1;
        CLOCK(entity)++;
        if (CLOCK(entity) >= walk[(s8)GAIT(entity) * 2 + 1])
        {
            CLOCK(entity) = 0;
            GAIT(entity)++;
            if ((s8)GAIT(entity) > 7) GAIT(entity) = 0;
        }
        FRAME(entity) = walk[(s8)GAIT(entity) * 2];
        if ((u16)entity->unk_4FC.part16.unk_4FE[0] == 7)
        {
            s32 facing;
            offset = entity->flipX ? 32 : -32;
            facing = entity->flipX;
            if (facing != Entity_IsTargetRight(entity) || TileAhead(entity, background, offset))
            {
                entity->subState = 2;
                CLOCK(entity) = 0;
                GAIT(entity) = 0;
                entity->velX.whole = 0;
                break;
            }
        }
        if (RandomNumberGenerator() % 100 <= 50)
        {
            s32 facing = entity->flipX << 6;
            offset = 80;
            if (facing) offset = 0;
            if (Target_IsInRect((s16)(entity->posX.part16.integer - offset),
                             (s16)(entity->posY.part16.integer - 24), 80, 24)
                && (u16)entity->unk_4FC.part16.unk_4FE[0] == 0x805)
            {
                entity->state = 1;
                entity->subState = 0;
                entity->timer = 0;
                entity->velX.whole = 0;
                break;
            }
        }
        if (entity->unk_500.part8.unk_500)
        {
            offset = entity->flipX ? -32 : 32;
            if (!TileAhead(entity, background, offset))
            {
                entity->subState = 3;
                CLOCK(entity) = 60;
                entity->velX.whole = 0;
                entity->timer = 0;
                break;
            }
        }
        Entity_MoveWithCollision(entity, 0x40000, -0x200000, 0x80000, 24);
        if (FRAME(entity) == 4 || FRAME(entity) == 7) entity->velX.whole = 0;
        else entity->velX.whole = entity->flipX ? 0x8000 : -0x8000;
        break;
    }
    case 2:
    {
        entity->unk_4FC.part8.unk_4FC |= 1;
        CLOCK(entity)++;
        if (CLOCK(entity) >= turn[(s8)GAIT(entity) * 2 + 1])
        {
            CLOCK(entity) = 0;
            GAIT(entity)++;
            if ((s8)GAIT(entity) > 5)
            {
                entity->subState = 1;
                GAIT(entity) = phase;
                entity->velX.whole = 0;
                entity->accelX.whole = 0;
                entity->accelY.whole = 0;
                entity->flipX ^= 1;
                entity->unk_4FC.part8.unk_4FC |= 1;
                FRAME(entity) = walk[(s8)GAIT(entity) * 2];
                break;
            }
        }
        FRAME(entity) = turn[(s8)GAIT(entity) * 2];
        entity->velX.whole = 0;
        if ((entity->unk_4FC.whole & 0xFF00FF00) == 0x100)
        {
            entity->accelX.whole = (gEwramData->playerState.player->posX.whole - entity->posX.whole) / 2;
            entity->velX.whole = -entity->accelX.whole;
            FRAME(entity)--;
        }
        if ((entity->unk_4FC.whole & 0xFF00FF00) == 0x200)
        {
            entity->accelY.whole = gEwramData->playerState.player->posX.whole - entity->posX.whole;
            entity->velX.whole = -entity->accelY.whole;
            FRAME(entity)--;
        }
        if ((entity->unk_4FC.whole & 0xFF00FF00) == 0x300)
        {
            entity->velX.whole = entity->flipX ? 0x10000 : -0x10000;
            FRAME(entity)--;
        }
        if ((entity->unk_4FC.whole & 0xFF00FF00) == 0x400)
        {
            entity->velX.whole = -(entity->accelX.whole + entity->accelY.whole);
            FRAME(entity)--;
        }
        if ((entity->unk_4FC.whole & 0xFF00FF00) == 0x500)
        {
            entity->velX.whole = entity->flipX ? -0x10000 : 0x10000;
            FRAME(entity)--;
        }
        break;
    }
    case 3:
        entity->unk_4FC.part8.unk_4FC |= 1;
        CLOCK(entity)++;
        if (CLOCK(entity) >= walk[(s8)GAIT(entity) * 2 + 1])
        {
            CLOCK(entity) = 0;
            GAIT(entity)--;
            if ((s8)GAIT(entity) < 0) GAIT(entity) = 7;
        }
        FRAME(entity) = walk[(s8)GAIT(entity) * 2];
        Entity_MoveWithCollision(entity, 0x40000, -0x200000, 0x80000, 24);
        if (FRAME(entity) == 4 || FRAME(entity) == 7)
        {
            entity->velX.whole = 0;
            if (entity->timer > 21)
            {
                entity->subState = 1;
                entity->accelX.whole = 0;
                entity->accelY.whole = 0;
                break;
            }
        }
        else entity->velX.whole = entity->flipX ? -0x8000 : 0x8000;
        if (++entity->timer > 59) entity->timer = 60;
        break;
    }
}
