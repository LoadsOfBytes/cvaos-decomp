#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code_08001194.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 sWarpRoomGfx[], sWarpRoomPalette[], sWarpRoomSpriteSheet[];
extern u8 sSaveWarpRoomGfx[], sSaveWarpRoomPalette[], sSaveWarpRoomSpriteSheet[];

struct RoomExit { u8 pad[4]; s8 direction; u8 rest[11]; };
struct ExitRoom { u8 pad[0x18]; struct RoomExit *exits; u16 count; };

static inline s32 RoomX(void) { return (s16)gEwramData->bgInfo[1].xPos.part16.integer; }
static inline s32 RoomY(void) { return (s16)gEwramData->bgInfo[1].yPos.part16.integer; }

static inline s32 ExitSum(void)
{
    struct ExitRoom *room; s32 sum, i;
    { struct GameState *ctx = &gEwramData->gameState; room = (struct ExitRoom *)ctx->currentRoomDefinition; }
    sum = 0;
    for (i = 0; i < room->count; i++)
        sum += room->exits[i].direction;
    return sum;
}

void Object1DCreate(struct EwramData_EntityData *entity)
{
    u8 *gfx, *palette, *sprite;
    s32 layer, graphics, sum, i;
    struct ExitRoom *room;
    struct RoomExit *exit;
    if (entity->unk_514.part16.spawnParam0 == 0)
    {
        gfx = sWarpRoomGfx;
        palette = sWarpRoomPalette;
        sprite = sWarpRoomSpriteSheet;
        layer = 0xC;
    }
    else
    {
        gfx = sSaveWarpRoomGfx;
        palette = sSaveWarpRoomPalette;
        sprite = sSaveWarpRoomSpriteSheet;
        layer = 0xA;
    }
    graphics = ObjGfx_Find(gfx);
    if (graphics < 0)
        graphics = ObjGfx_LoadResource(gfx, 0);
    ObjPalette_LoadRange(palette, 0, 1, layer);
    Sprite_Attach(entity, sprite, graphics, layer);
    entity->spriteLayer = 0x11;
    sum = ExitSum();
    if (entity->unk_514.part16.spawnParam0 == 0)
    {
        entity->animFrame = 0xB;
        if (sum == 1)
        {
            entity->posX.part16.integer -= 0xD0;
            entity->ext.object1D.exitRight = 1;
            entity->flipX = 0;
        }
        else if (sum == -1)
        {
            entity->ext.object1D.exitRight = 0;
            entity->flipX = 1;
        }
        else if (sum == 0)
        {
            EntityDelete(entity);
            return;
        }
    }
    else
    {
        entity->animFrame = 0x14;
        if (sum == 1)
        {
            entity->posX.part16.integer = 0x10;
            entity->posY.part16.integer = 0x70;
            entity->ext.object1D.exitRight = 1;
            entity->flipX = 0;
        }
        else if (sum == -1)
        {
            entity->posX.part16.integer = 0xE0;
            entity->posY.part16.integer = 0x70;
            entity->ext.object1D.exitRight = 0;
            entity->flipX = 1;
        }
        else if (sum == 0)
        {
            EntityDelete(entity);
            return;
        }
    }
    for (i = 0; i <= 0x30; i += 0x10)
    {
        s32 x = (s16)entity->posX.part16.integer;
        struct BgLayerState *bg = &gEwramData->bgInfo[1];
        s32 y;
        x += (s16)bg->xPos.part16.integer;
        x -= entity->flipX * 16;
        y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
        Collision_SetBlock(x, y - i);
    }
}
