#include "code_08009A0.h"
#include "code_0800B700.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief Unreferenced part next to the warp room code: once its parent leaves
 * state 0 it rises, spins and converges on the screen centre while
 * shrinking; states 10/11 drop it back to y 0x84 and re-enable the
 * menus.
 */
void WarpRoomUnusedPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.warpRoomUnusedPart.parent;
    s32 limit;

    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    switch (entity->state)
    {
    case 0:
        if (parent->state == 0)
            break;
        entity->timer = (RandomNumberGenerator() & 0xF) + 8;
        entity->state++;
        break;
    case 1:
        if (entity->timer != 0)
        {
            entity->timer--;
            entity->posY.whole -= 0x10000;
            break;
        }
        entity->timer = (RandomNumberGenerator() & 0xF) + 0x10;
        entity->state++;
        break;
    case 2:
        entity->ext.warpRoomUnusedPart.angle += 0x400;
        if (--entity->timer != 0)
            break;
        entity->velX.whole = -(entity->posX.whole >> 6);
        entity->velY.whole = (entity->posX.whole - 0xA00000) >> 7;
        entity->state++;
        break;
    case 3:
        limit = 0x30000 - (entity->ext.warpRoomUnusedPart.riseFrame << 9);
        if (limit < 0)
            limit = 0;
        if (entity->posX.part16.integer < 0x80)
        {
            entity->velX.whole += 0x4000;
            if (entity->velX.whole > limit)
                entity->velX.whole = limit;
        }
        else
        {
            entity->velX.whole -= 0x4000;
            if (entity->velX.whole < -limit)
                entity->velX.whole = -limit;
        }
        if ((u16)entity->posY.part16.integer < 0x50)
        {
            entity->velY.whole += 0x3000;
            if (entity->velY.whole > limit)
                entity->velY.whole = limit;
        }
        else
        {
            entity->velY.whole -= 0x3000;
            if (entity->velY.whole < -limit)
                entity->velY.whole = -limit;
        }
        entity->ext.warpRoomUnusedPart.riseFrame++;
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->ext.warpRoomUnusedPart.scale -= 0x200;
        if (entity->ext.warpRoomUnusedPart.scale == 0)
            entity->state = 0x63;
        break;
    case 10:
        entity->ext.warpRoomUnusedPart.angle += 0x400;
        if (++entity->timer > 0x2F)
            entity->state++;
        break;
    case 11:
        entity->velY.whole += 0x6000;
        entity->posY.whole += entity->velY.whole;
        if ((u16)entity->posY.part16.integer > 0x83)
        {
            if (entity->unk_518.part8.spawnIndex == 0)
                Game_SetMenusEnabled(1);
            entity->posY.part16.integer = 0x84;
            entity->state = 0;
        }
        break;
    case 0x63:
        entity->deleteRequested = 1;
        ObjAffine_Free(entity->ext.warpRoomUnusedPart.affine);
        return;
    }
    ObjAffine_Set(entity->ext.warpRoomUnusedPart.angle, entity->ext.warpRoomUnusedPart.scale, entity->ext.warpRoomUnusedPart.scale, 0, 0, entity->ext.warpRoomUnusedPart.affine);
}
