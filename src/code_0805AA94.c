#include "code/code_08039340.h"
#include "room_object_entity.h"
#include "code_08009A0.h"
#include "code_08001194.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

extern u8 Object37_LoadGraphics(struct EwramData_EntityData *);
extern void Object37_SpawnChild(struct EwramData_EntityData *);

/* Keep the entity and room coordinate reads in their original order. */
static inline s32 EntityRoomX(struct EwramData_EntityData *entity, struct EwramData **ram)
{
    s32 x = (s16)entity->posX.part16.integer;
    struct BgLayerState *bg = &(*ram)->bgInfo[1];
    return x + (s16)bg->xPos.part16.integer;
}

static inline s32 EntityRoomY(struct EwramData_EntityData *entity, struct EwramData **ram)
{
    s32 y = (s16)entity->posY.part16.integer;
    struct BgLayerState *bg = &(*ram)->bgInfo[1];
    return y + (s16)bg->yPos.part16.integer;
}

static inline s32 Abs(s32 value)
{
    if (value < 0)
        value = -value;
    return value;
}

/* Keep the mask value (0 or 0x40), as used by the original condition. */
static inline u8 FlipMask(struct EwramData_EntityData *entity)
{
    return ((u8 *)entity)[0x58] & 0x40;
}

/**
 * @brief Main behaviour of object 0x37, called by Object37Update.
 */
void Object37_UpdateState(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player = gEwramData->playerState.player;
    s32 distance = entity->posX.whole - player->posX.whole;
    s32 x, y;
    switch (entity->subState)
    {
    case 0:
        if ((s32)entity->ext.object37.angle > 0)
        {
            entity->ext.object37.angle -= 0x100;
            if ((s32)entity->ext.object37.angle < 0)
                entity->ext.object37.angle = 0;
        }
        if (Abs(distance) >= 0x300000 && FlipMask(player))
        {
            entity->subState = 1;
            PlaySong(0x113);
        }
        break;
    case 1:
        if ((s32)entity->ext.object37.angle < 0x3000)
        {
            entity->ext.object37.angle += 0x100;
            if ((s32)entity->ext.object37.angle > 0x3000)
                entity->ext.object37.angle = 0x3000;
        }
        else if (entity->ext.object37.closed != 0)
        {
            entity->ext.object37.closed = 0;
            x = EntityRoomX(entity, &gEwramData);
            y = EntityRoomY(entity, &gEwramData);
            x -= 0x10;
            Collision_ClearBlock(x, y + 8);
            Collision_ClearBlock(x, y - 8);
            Collision_ClearBlock(x, y - 0x18);
            Collision_ClearBlock(x, y - 0x28);
            Collision_ClearBlock(x, y - 0x38);
        }
        if (Abs(distance) >= 0x300000 && !FlipMask(player))
        {
            entity->subState = 0;
            entity->ext.object37.closed = 1;
{
                s32 px = (s16)entity->posX.part16.integer;
                struct BgLayerState *bg = &gEwramData->bgInfo[1];
                x = px + (s16)bg->xPos.part16.integer;
                y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
            }
            x -= 0x10;
            Collision_SetBlock(x, y + 8);
            Collision_SetBlock(x, y - 8);
            Collision_SetBlock(x, y - 0x18);
            Collision_SetBlock(x, y - 0x28);
            Collision_SetBlock(x, y - 0x38);
            PlaySong(0x113);
        }
        break;
    }
    ObjAffine_Set(entity->ext.object37.angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
}
