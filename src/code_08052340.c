#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code_080D73B8.h"
#include "code_08013960.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
extern void Object14_Draw(struct EwramData_EntityData *);
extern void Object14_MoveOnTerrainSolid(struct EwramData_EntityData *, s32);
extern void Object14_SpawnDebris(struct EwramData_EntityData *);
extern s32 Entity_DistanceToPoint(struct EwramData_EntityData *, s32, s32);
extern void Angle_TurnTowards(s16 *, s32, s32);
extern s32 Sin(s32);
extern s32 GetEntityRoomXPositionInteger(struct EwramData_EntityData *);
extern s32 GetEntityRoomYPositionInteger(struct EwramData_EntityData *);
extern s32 Collision_GetPushLeftFiltered(s32, s32, s32);
extern u8 sObject14Variant1SolidCallbacks[];


static inline struct EwramData_EntityData *Player(struct EwramData **ew)
{
    return (*ew)->playerState.player;
}

/**
 * @brief Update of object 0x14 variant 1 (sObject14VariantFuncs[1]): rolling affine
 * sprite with a body hit box, registered as solid object
 * (sObject14Variant1SolidCallbacks).
 */
void Object14Variant1_Update(struct EwramData_EntityData *entity)
{
    s32 length, x, y;
    switch (entity->state) {
    case 0: {
        s16 slot = ObjAffine_Alloc(1);
        if (slot < 0)
            goto delete;
        entity->hidden = 0;
        entity->spriteLayer = 12;
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->drawFunc = (u32 *)Object14_Draw;
        entity->animFrame = 2;
        entity->ext.object14.roomX = GetEntityRoomXPositionInteger(entity);
        entity->ext.object14.roomY = GetEntityRoomYPositionInteger(entity);
        entity->ext.object14.prevX = entity->ext.object14.roomX;
        entity->ext.object14.prevY = entity->ext.object14.roomY;
        Hit_Enable(entity, 8, 0, 0);
        HitBox_SetBodyRaw(entity, 0x102EFEE8);
        entity->hit.flags |= 2;
        entity->hit.flags |= 4;
        Player_RegisterSolidObject(entity, sObject14Variant1SolidCallbacks);
        entity->state = 1;
    }
    case 1:
        if (entity->ext.object14.flags & 1) {
            if (entity->velX.whole == 0)
                Sound_PlayTracked(0x136);
            entity->stage++;
            entity->velX.whole = 0x4000;
        } else {
            if (entity->velX.whole != 0)
                Sound_StopTracked(0x136);
            entity->velX.whole = 0;
        }
        Object14_MoveOnTerrainSolid(entity, 0);
        if (entity->ext.object14.angle != 0) {
            Sound_PlayTracked(0x136);
            entity->ext.object14.angle = 0;
            entity->state = 2;
            entity->timer = 8;
        }
        break;
    case 2:
        if (entity->timer && (u8)--entity->timer == 0)
            Player_ClearSolidObject();
        entity->stage++;
        if (entity->velX.whole <= 0x7FFFF)
            entity->velX.whole += entity->ext.object14.angle / 2;
        entity->velY.whole = entity->velX.whole;
        Object14_MoveOnTerrainSolid(entity, 0);
        if (entity->ext.object14.angle < 0) {
            Sound_StopTracked(0x136);
            entity->state = 3;
            entity->accelY.whole = 0x400;
            entity->velY.whole += -0x74000;
        }
        break;
    case 3: {
        s32 x, y;
        if (entity->velX.whole <= 0x7FFFF)
            entity->velX.whole += 0x800;
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += entity->accelY.whole;
        Player_QueueEntity(entity, 7);
        Angle_TurnTowards(&entity->ext.object14.angle, 0x80, 0xE000);
        x = GetEntityRoomXPositionInteger(entity) + 26;
        y = GetEntityRoomYPositionInteger(entity);
        if (Collision_GetPushLeftFiltered(x, y, 0)) {
            Object14_SpawnDebris(entity);
            PlaySong(0xAC);
            ObjAffine_Free(entity->affineIndex);
delete:
            EntityDelete(entity);
            return;
        }
        break;
    }
    }
    entity->ext.object14.flags = 0;
    ObjAffine_Set(entity->ext.object14.angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
    if (entity->state > 1) {
        length = Entity_DistanceToPoint(entity, Player(&gEwramData)->posX.whole,
                            Player(&gEwramData)->posY.whole);
        if (length > 0xC0000)
            length = 0xC0000;
        if (Player(&gEwramData)->posX.whole < entity->posX.whole)
            length = -length;
        x = (length < 0 ? -(-length >> 16) : length >> 16) * Sin(entity->ext.object14.angle + 0x4000);
        y = Sin(entity->ext.object14.angle) * (length < 0 ? -(-length >> 16) : length >> 16);
        HitBox_SetBodyRaw(entity, (((x < 0 ? -(-x >> 16) : x >> 16) - 8) & 0xFF) + (((y < 0 ? -(-y >> 16) : y >> 16) & 0xFF) << 8) + 0x14140000);
    }
}
