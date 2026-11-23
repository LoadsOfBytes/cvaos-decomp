#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "syscalls.h"
extern s32 Enemy_IsStunnedNoFlash(struct EwramData_EntityData *);
extern s32 Sin(s32);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);

#define FIXED_TO_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)

/**
 * @brief 7DE14 | Second Giant Ghost hand: like GiantGhostHandA_Update but at 2x sine
 * speed, swaying around 4 / 0x24 px from the owner and reaching for the
 * player from 4 px in front of / 4 px above the owner.
 */
void GiantGhostHandB_Update(struct EwramData_EntityData *entity)
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
            if (entity->ext.giantGhostHand.swayDir)
            {
                entity->ext.giantGhostHand.pointX -= 0x10000;
                entity->ext.giantGhostHand.pointY -= 0x10000;
                if ((s8)++entity->ext.giantGhostHand.swayFrame > 0x3B)
                {
                    entity->ext.giantGhostHand.swayFrame = 0;
                    entity->ext.giantGhostHand.swayDir ^= 1;
                }
            }
            else
            {
                entity->ext.giantGhostHand.pointX += 0x10000;
                entity->ext.giantGhostHand.pointY += 0x10000;
                if ((s8)++entity->ext.giantGhostHand.swayFrame > 0x3B)
                {
                    entity->ext.giantGhostHand.swayFrame = 0;
                    entity->ext.giantGhostHand.swayDir ^= 1;
                }
            }
            x = parent->posX.whole - 0x40000;
            y = parent->posY.whole - 0x240000;
            x = entity->ext.giantGhostHand.pointX - x;
            y = entity->ext.giantGhostHand.pointY - y;
            angle = ArcTan2(FIXED_TO_INT(x), FIXED_TO_INT(y));
            x = Sin(angle + 0x4000);
            y = Sin(angle);
            entity->velX.whole = x * 2;
            entity->velY.whole = y * 2;
            if (entity->flipX)
                entity->velX.whole = x * 2 - 0x10000;
            else
                entity->velX.whole = x * 2 + 0x10000;
        }
        else if (!Enemy_IsStunnedNoFlash(parent))
        {
            if (entity->flipX)
                x = parent->posX.whole + 0x40000;
            else
                x = parent->posX.whole - 0x40000;
            y = parent->posY.whole - 0x40000;
            x = Target_GetX(entity) - x;
            y = Target_GetY(entity) - y;
            angle = ArcTan2(FIXED_TO_INT(x), FIXED_TO_INT(y));
            x = Sin(angle + 0x4000);
            y = Sin(angle);
            entity->velX.whole = x * 2;
            entity->velY.whole = y * 2;
            if (entity->flipX)
                entity->velX.whole = x * 2 - 0x10000;
            else
                entity->velX.whole = x * 2 + 0x10000;
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
