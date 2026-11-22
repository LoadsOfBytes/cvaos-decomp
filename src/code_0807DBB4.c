#include "code_08009A0.h"
#include "giant_ghost_entity.h"
#include "code/code_08039340.h"
#include "syscalls.h"
extern s32 Enemy_IsStunnedNoFlash(struct EwramData_EntityData *);
extern s32 Sin(s32);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);

#define FIXED_TO_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)

/**
 * @brief 7DBB4 | Giant Ghost hand (owner type 0x13 in unk_51A): copies the
 * owner's visibility, palette and facing; while gEwramData bestiaryPreview is set
 * it sways a target point (unk_4FC / unk_500, direction unk_506 flipped
 * every 60 frames) and moves towards it at 3x sine speed from the owner
 * (1 / 0x1E px above), otherwise (unless Enemy_IsStunnedNoFlash) it reaches for the
 * player (Target_GetX / Target_GetY) from 1 px in front of the owner.
 * Vanishes (deleteRequested) when the owner enters state 2.
 */
void GiantGhostHandA_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    s32 x;
    s32 y;
    u16 angle;
    s32 c;
    s32 s;

    if (entity->state == 0)
    {
        if (!parent || !parent->updateFunc || parent->unk_518.part8.enemyId != 0x13)
            goto remove;
        entity->hidden = parent->hidden;
        entity->palette = parent->palette;
        entity->flipX = parent->flipX;
        if (gEwramData->gameState.bestiaryPreview)
        {
            if (GIANT_GHOST_HAND_SWAY_DIR(entity))
            {
                GIANT_GHOST_HAND_POINT_X(entity) -= 0x10000;
                GIANT_GHOST_HAND_POINT_Y(entity) -= 0x10000;
                if ((s8)++GIANT_GHOST_HAND_SWAY_FRAME(entity) > 0x3B)
                {
                    GIANT_GHOST_HAND_SWAY_FRAME(entity) = 0;
                    GIANT_GHOST_HAND_SWAY_DIR(entity) ^= 1;
                }
            }
            else
            {
                GIANT_GHOST_HAND_POINT_X(entity) += 0x10000;
                GIANT_GHOST_HAND_POINT_Y(entity) += 0x10000;
                if ((s8)++GIANT_GHOST_HAND_SWAY_FRAME(entity) > 0x3B)
                {
                    GIANT_GHOST_HAND_SWAY_FRAME(entity) = 0;
                    GIANT_GHOST_HAND_SWAY_DIR(entity) ^= 1;
                }
            }
            x = parent->posX.whole - 0x10000;
            y = parent->posY.whole - 0x1E0000;
            x = GIANT_GHOST_HAND_POINT_X(entity) - x;
            y = GIANT_GHOST_HAND_POINT_Y(entity) - y;
            angle = ArcTan2(FIXED_TO_INT(x), FIXED_TO_INT(y));
            x = Sin(angle + 0x4000);
            y = Sin(angle);
            entity->velX.whole = x * 3;
            entity->velY.whole = y * 2;
            if (entity->flipX)
                entity->velX.whole = x * 3 - 0x10000;
            else
                entity->velX.whole = x * 3 + 0x10000;
        }
        else if (!Enemy_IsStunnedNoFlash(parent))
        {
            if (entity->flipX)
                x = parent->posX.whole + 0x10000;
            else
                x = parent->posX.whole - 0x10000;
            y = parent->posY.whole - 0x10000;
            x = Target_GetX(entity) - x;
            y = Target_GetY(entity) - y;
            angle = ArcTan2(FIXED_TO_INT(x), FIXED_TO_INT(y));
            x = Sin(angle + 0x4000);
            y = Sin(angle);
            entity->velX.whole = x * 3;
            entity->velY.whole = y * 2;
            if (entity->flipX)
                entity->velX.whole = x * 3 - 0x10000;
            else
                entity->velX.whole = x * 3 + 0x10000;
        }
        entity->posX.whole = parent->posX.whole + entity->velX.whole;
        entity->posY.whole = parent->posY.whole + entity->velY.whole;
        if (parent->state == 2)
        {
            entity->state = 1;
            entity->deleteRequested = 1;
        }
        return;
    remove:
        EntityDelete(entity);
        return;
    }
    entity->deleteRequested = 1;
}
