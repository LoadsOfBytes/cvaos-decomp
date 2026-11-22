/* Objects 0x35 / 0x36 helpers 0x0805A590..0x0805A747. */
#include "gba.h"
#include "syscalls.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08050A3C.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Player_QueueEntity(struct EwramData_EntityData *entity, u8 collisionFlags);
extern void Object35_UpdateState(struct EwramData_EntityData *entity);
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern s32 Player_RegisterSolidObject(struct EwramData_EntityData *entity, void *callbackTable);
void Object35_Draw(struct EwramData_EntityData *entity);

extern u8 sObject35Gfx[];
extern u8 sObject35Palette[];
extern u8 sObject35SpriteSheet[];
extern u8 sObject36SolidCallbacks[];

/* Per variant: { rotation speed * 2 / scale step, rotation speed }. */
extern const s16 sObject36Shapes[9][2];

/**
 * @brief 5A590 | Object 0x35 update: Player_QueueEntity(.., 1) while unk_514 is 0,
 * Object35_UpdateState in state 0.
 */
void Object35Update(struct EwramData_EntityData *entity)
{
    u8 hitInfo[0x10];

    if (entity->unk_514.part16.spawnParam0 == 0)
        Player_QueueEntity(entity, 1);
    if (entity->state == 0)
        Object35_UpdateState(entity);
}

/**
 * @brief 5A5BC | Nothing.
 */
void Object35Child_Update(void)
{
}

/**
 * @brief 5A5C0 | Draw; when unk_500 is set first draw a copy as OBJ window
 * (0x53E mode 2) at y = unk_4F8 + 0x30.
 */
void Object35_Draw(struct EwramData_EntityData *entity)
{
    s32 oldY = entity->posY.whole;

    if (entity->unk_500.whole != 0)
    {
        entity->objMode = 2;
        gUnk_03002CB0.dispCnt |= DCNT_WINOBJ;
        entity->posY.whole = entity->unk_4F8.whole + 0x300000;
        Sprite_Draw(entity);
        entity->objMode = 0;
    }
    entity->posY.whole = oldY;
    Sprite_Draw(entity);
}

/**
 * @brief 5A618 | Load the graphics (sObject35Gfx), palette and animation
 * set sObject35SpriteSheet; drawFunc Object35_Draw when unk_514 is set.
 */
u8 Object35_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject35Gfx, 0);

    if (gfx < 0)
        return 0;

    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sObject35Palette, 0);
    Sprite_Attach(entity, sObject35SpriteSheet, gfx, entity->unk_510.part8.basePalette);
    if (entity->unk_514.part16.spawnParam0 != 0)
        entity->drawFunc = (u32 *)Object35_Draw;
    else
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 5A678 | Object 0x36 update: rotate BG2 (angle unk_4F8, scale
 * unk_500) around the entity position, then advance the angle by unk_4FE.
 */
void Object36Update(struct EwramData_EntityData *entity)
{
    struct BgAffineSrcData affineSrc;

    affineSrc.texX = 0x8000;
    affineSrc.texY = 0x8000;
    affineSrc.scrX = entity->posX.part16.integer;
    affineSrc.scrY = entity->posY.part16.integer;
    affineSrc.sx = entity->unk_500.part16.unk_500;
    affineSrc.sy = entity->unk_500.part16.unk_500;
    affineSrc.alpha = entity->unk_4F8.whole;
    BgAffineSet(&affineSrc, (struct BgAffineDstData *)&gDisplayRegisters.bg2PA, 1);
    entity->unk_4F8.whole += entity->unk_4FC.part16.unk_4FE[0];
}

/**
 * @brief 5A6C8 | Object 0x36 create: rotation speed and scale from the
 * sObject36Shapes entry selected by unk_514, then Player_RegisterSolidObject.
 */
void Object36Create(struct EwramData_EntityData *entity)
{
    s16 shapes[9][2];

    memcpy(shapes, sObject36Shapes, sizeof(shapes));
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    entity->hidden = 1;
    entity->unk_4FC.part16.unk_4FC = shapes[entity->unk_514.part16.spawnParam0][0] / 2;
    entity->unk_4FC.part16.unk_4FE[0] = shapes[entity->unk_514.part16.spawnParam0][1];
    entity->unk_500.part16.unk_500 = 0x200 - shapes[entity->unk_514.part16.spawnParam0][0] * 2;
    Player_RegisterSolidObject(entity, sObject36SolidCallbacks);
}
