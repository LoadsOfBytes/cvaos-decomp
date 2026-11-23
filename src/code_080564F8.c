/* Objects 0x1F / 0x21 create, 0x080564F8..0x08056617. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08050A3C.h"
#include "structs/ewram.h"
/* ObjGfx slot of sWarpRoomGfx; stored, never read. */
#define WARP_ROOM_GFX(e) e->unk_4FC.part16.unk_4FC

extern void WarpRoom_Update(struct EwramData_EntityData *e);
extern void WarpRoom_CreateGateParts(struct EwramData_EntityData *owner);
extern void WarpRoom_CreateSideLights(struct EwramData_EntityData *owner);
extern u8 Object21_LoadGraphics(struct EwramData_EntityData *entity);
extern void Object21_OnPlayerContact(struct EwramData_EntityData *entity);
s32 WarpRoom_LoadGraphics(struct EwramData_EntityData *entity);

extern u8 sWarpRoomGfx[];

/**
 * @brief 564F8 | Object 0x1F create.
 */
void Object1FCreate(struct EwramData_EntityData *entity)
{
    WarpRoom_LoadGraphics(entity);
    WarpRoom_CreateGateParts(entity);
    WarpRoom_CreateSideLights(entity);
}

/**
 * @brief Get (or load) the graphics set sWarpRoomGfx into unk_4FC and switch
 * the update to WarpRoom_Update; delete the entity when that fails
 * (no return value then).
 */
s32 WarpRoom_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_Find(sWarpRoomGfx);

    if (gfx < 0)
    {
        gfx = ObjGfx_LoadResource(sWarpRoomGfx, 0);
        if (gfx < 0)
        {
            EntityDelete(entity);
            return;
        }
    }
    WARP_ROOM_GFX(entity) = gfx;
    entity->updateFunc = (u32 *)WarpRoom_Update;
    return 1;
}

/**
 * @brief 56550 | Object 0x21 create: hitbox with callback Object21_OnPlayerContact,
 * strength attack from the player stats (equipmentDefenseBonus * 3 / 4 + maxHP / 16),
 * fixed room position (0x40, 0x18).
 */
void Object21Create(struct EwramData_EntityData *entity)
{
    struct EwramData **ewramRef = &gEwramData;
    struct BgLayerState *bg = &(*ewramRef)->bgInfo[1];
    struct SpriteBox box = { -30, 41, 0x3C, 0x14 };

    if (!Object21_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Hit_Enable(entity, 8, Object21_OnPlayerContact, 0);
    HitBox_SetAttack(entity, &box);
    entity->hit.flags |= 4;
    entity->hit.flags &= ~2;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    {
        s32 power = ((*ewramRef)->playerState.equipmentDefenseBonus >> 1) + ((*ewramRef)->playerState.equipmentDefenseBonus >> 2);

        power += (*ewramRef)->playerStatus.maxHP >> 4;
        entity->attack = power;
    }
    entity->spriteLayer = 0x12;
    entity->state = 0;
    entity->posX.part16.integer = 0x40 - bg->xPos.part16.integer;
    entity->posY.part16.integer = 0x18 - bg->yPos.part16.integer;
    entity->accelY.whole = 0x2800;
}
