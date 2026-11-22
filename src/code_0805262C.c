/* Object14Variant2_Update. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s16 GetEntityRoomXPositionInteger(struct EwramData_EntityData *entity);
extern s16 GetEntityRoomYPositionInteger(struct EwramData_EntityData *entity);
extern s32 Collision_GetGroundOffset(s32 x, s32 y);
extern s32 Collision_IsPointSolid(s32 x, s32 y);
extern void EntityDampVelocityX(struct EwramData_EntityData *entity, s32 factor);
extern void Object14_Draw(struct EwramData_EntityData *entity);

static inline u16 RoomX(struct EwramData_EntityData *entity)
{
    u16 x = entity->posX.part16.integer;
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    return bg->xPos.part16.integer + x;
}

static inline u16 RoomY(struct EwramData_EntityData *entity)
{
    u16 y = entity->posY.part16.integer;
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    return bg->yPos.part16.integer + y;
}

/**
 * @brief Update of object 0x14 variant 2 (sObject14VariantFuncs[2]): carries the player
 * on top while moving.
 */
void Object14Variant2_Update(struct EwramData_EntityData *entity)
{
    s16 x;
    s16 y;
    struct BgLayerState *bg;
    s32 probeX;
    s32 probeY;
    s32 speed;

    if (Entity_IsInRect(entity, -0x80, -0x40, 0x1F0, 0x120) != 0)
    {
        Player_QueueEntity(entity, 3);
        if (gEwramData->playerState.standingPlatform == entity)
            Player_SetXFraction((s16)entity->posX.part16.fraction);
    }
    switch (entity->state)
    {
    case 0:
        entity->hidden = 0;
        entity->spriteLayer = 0xC;
        entity->animFrame = 2;
        entity->drawFunc = (u32 *)Object14_Draw;
        entity->unk_4FC.part16.unk_4FC = GetEntityRoomXPositionInteger(entity);
        entity->unk_4FC.part16.unk_4FE[0] = GetEntityRoomYPositionInteger(entity);
        entity->unk_500.part16.unk_500 = entity->unk_4FC.part16.unk_4FC;
        entity->unk_500.part16.unk_502 = entity->unk_4FC.part16.unk_4FE[0];
        Hit_Enable(entity, 8, NULL, 0);
        HitBox_SetBodyRaw(entity, 0x103000E8);
        entity->hit.flags |= 2;
        entity->hit.flags |= 4;
        if (RandomNumberGenerator() & 1)
            entity->unk_508.part16.unk_508 = 0x8000;
        entity->unk_508.part16.unk_50A = (RandomNumberGenerator() & 0x3F) + 0x40;
        entity->state = 1;
    case 1:
        entity->unk_508.part16.unk_508 += entity->unk_508.part16.unk_50A;
        entity->velX.whole = Sin(entity->unk_508.part16.unk_508) / 3;
        entity->posX.whole += entity->velX.whole;
        break;
    case 2:
        x = entity->velX.part16.integer + RoomX(entity);
        if (entity->velX.whole < 0)
            x -= 0x18;
        else
            x += 0x18;
        { u32 tmp_ = RoomY(entity) << 16; y = (s32)tmp_ >> 16; }
        if (Collision_GetGroundOffset(x, y) && !Collision_IsPointSolid(x, y))
            entity->velX.whole = -entity->velX.whole / 2;
        EntityDampVelocityX(entity, 0x50);
        EntityAccelerateAndMove(entity, 0x20000);
        speed = entity->velX.whole;
        if (speed < 0)
            speed = -speed;
        if (speed <= 0x7FF)
            entity->state = 1;
        break;
    }
}
