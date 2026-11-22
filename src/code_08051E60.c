/* Entity with rise/fall states whose affine sprite is scaled and turned by the player distance. */
#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
extern void Object14_Draw(struct EwramData_EntityData *);
extern s32 Object14_MoveOnTerrain(struct EwramData_EntityData *, s32);
extern void Object14_SpawnDebris(struct EwramData_EntityData *);
extern s32 Entity_IsInRect(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 Entity_DistanceToPoint(struct EwramData_EntityData *, s32, s32);
extern s32 Sin(s32);
extern s32 GetEntityRoomXPositionInteger(struct EwramData_EntityData *);
extern s32 GetEntityRoomYPositionInteger(struct EwramData_EntityData *);
static inline struct EwramData_EntityData *Player(struct EwramData **ew)
{
    return (*ew)->playerState.player;
}

static inline s32 PackY(s32 value, u8 mask)
{
    return ((value < 0 ? -(-value >> 16) : value >> 16) & mask) << 8;
}

/**
 * @brief Update of object 0x14 variant 0 (sObject14VariantFuncs[0]): affine sprite that
 * rolls along the terrain from its start position.
 */
void Object14Variant0_Update(struct EwramData_EntityData *entity)
{
    s32 length, x, y, radius;
    if (gEwramData->gameState.timeStopTimer != 0) {
        if (entity->state > 1)
            Player_QueueEntity(entity, 0);
        return;
    }
    switch (entity->state) {
    case 0: {
        s16 slot = ObjAffine_Alloc(1);
        if (slot < 0) {
            EntityDelete(entity);
            return;
        }
        entity->hidden = 0;
        entity->spriteLayer = 12;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->drawFunc = (u32 *)Object14_Draw;
        OBJECT14_ROOM_X(entity) = GetEntityRoomXPositionInteger(entity);
        OBJECT14_ROOM_Y(entity) = GetEntityRoomYPositionInteger(entity);
        OBJECT14_PREV_X(entity) = OBJECT14_ROOM_X(entity);
        OBJECT14_PREV_Y(entity) = OBJECT14_ROOM_Y(entity);
        entity->velX.whole = 0x8000;
        Hit_Enable(entity, 8, 0, 0);
        HitBox_SetBodyRaw(entity, 0x103000E8);
        entity->hit.flags |= 2;
        entity->hit.flags |= 4;
        entity->state = 1;
    }
    case 1:
        entity->animFrame = 0;
        OBJECT14_SCALE(entity) += 0x400;
        if (OBJECT14_SCALE(entity) > 0xFFFF) {
            entity->animFrame = 2;
            OBJECT14_SCALE(entity) = 0x10000;
            entity->state = 2;
        }
        break;
    case 2:
        if (!Object14_MoveOnTerrain(entity, 2)) {
            entity->state = 3;
            entity->accelY.whole = 0x2000;
        }
        break;
    case 3:
        if (Object14_MoveOnTerrain(entity, 1)) {
            if (entity->velY.whole > 0x40000)
                entity->state = 4;
            else {
                s32 zero = 0;
                entity->state = 2;
                entity->velY.whole = zero;
                entity->accelY.whole = zero;
            }
        }
        break;
    case 4: {
        struct BgLayerState *bg;
        if (Entity_IsInRect(entity, -16, 0, 0x110, 0xB0)) {
            Object14_SpawnDebris(entity);
            PlaySong(0xAC);
        }
        OBJECT14_SCALE(entity) = 0;
        OBJECT14_ANGLE(entity) = 0;
        entity->velY.whole = 0;
        entity->accelY.whole = 0;
        bg = &gEwramData->bgInfo[1];
        entity->posX.part16.integer = OBJECT14_ROOM_X(entity) - bg->xPos.part16.integer;
        entity->posY.part16.integer = OBJECT14_ROOM_Y(entity) - bg->yPos.part16.integer;
        entity->state = 1;
        break;
    }
    }
    length = Entity_DistanceToPoint(entity, Player(&gEwramData)->posX.whole, Player(&gEwramData)->posY.whole);
    if (length > 0xC0000)
        length = 0xC0000;
    if (Player(&gEwramData)->posX.whole < entity->posX.whole)
        length = -length;
    radius = OBJECT14_ANGLE(entity) == 0 ? 10 : 12;
    x = (length < 0 ? -(-length >> 16) : length >> 16) * Sin(OBJECT14_ANGLE(entity) + 0x4000);
    y = (length < 0 ? -(-length >> 16) : length >> 16) * Sin(OBJECT14_ANGLE(entity));
    { s32 left = -radius;
      length = radius << 16;
      if (x < 0) left -= -x >> 16; else left += x >> 16;
      left &= 0xFF;
      { register u8 mask asm("r3") = 0xFF; /* FAKEMATCH: pinned register */
      length <<= 1;
      { s32 sum = left + PackY(y, mask) + length; { s32 k = 0x14000000; asm("" :: "r"(k)); left = sum + k; } HitBox_SetBodyRaw(entity, left); } } } /* FAKEMATCH: k stays a pseudo (no combine/equiv fold), sum chain stays in r0 */
    ObjAffine_Set(OBJECT14_ANGLE(entity), OBJECT14_SCALE(entity), OBJECT14_SCALE(entity),
                entity->flipX, 0, entity->affineIndex);
}
