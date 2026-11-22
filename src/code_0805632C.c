#include "code_08009A0.h"
#include "code_0800B700.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/*
 * Unused warp room part (WarpRoomUnusedPart_Update): affine angle, rise frame,
 * scale (shrinks to 0) and its affine slot. Token-identical expansion.
 */
#define WARP_PART_ANGLE(e) e->unk_4F4.whole
#define WARP_PART_RISE_FRAME(e) e->unk_4F8.part16.unk_4F8
#define WARP_PART_SCALE(e) e->unk_500.whole
#define WARP_PART_AFFINE(e) e->unk_4F8.part8.unk_4FB

/**
 * @brief Unreferenced part next to the warp room code: once its parent leaves
 * state 0 it rises, spins and converges on the screen centre while
 * shrinking; states 10/11 drop it back to y 0x84 and re-enable the
 * menus.
 */
void WarpRoomUnusedPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->unk_4FC.unk_4FC_entity;
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
        WARP_PART_ANGLE(entity) += 0x400;
        if (--entity->timer != 0)
            break;
        entity->velX.whole = -(entity->posX.whole >> 6);
        entity->velY.whole = (entity->posX.whole - 0xA00000) >> 7;
        entity->state++;
        break;
    case 3:
        limit = 0x30000 - (WARP_PART_RISE_FRAME(entity) << 9);
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
        WARP_PART_RISE_FRAME(entity)++;
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        WARP_PART_SCALE(entity) -= 0x200;
        if (WARP_PART_SCALE(entity) == 0)
            entity->state = 0x63;
        break;
    case 10:
        WARP_PART_ANGLE(entity) += 0x400;
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
        ObjAffine_Free(WARP_PART_AFFINE(entity));
        return;
    }
    ObjAffine_Set(WARP_PART_ANGLE(entity), WARP_PART_SCALE(entity), WARP_PART_SCALE(entity), 0, 0, WARP_PART_AFFINE(entity));
}
