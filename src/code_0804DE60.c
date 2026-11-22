/* Helpers 0x0804DE60..0x0804DFAB. */
#include "gba.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Gate_SetupGraphics(struct EwramData_EntityData *entity, s32 opened);
void Door_Draw(struct EwramData_EntityData *entity);

extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];

/* Event flag param_1 of the bit array gameFlagsTail (as in Object35Create). */
#define OBJECT_FLAG(ewram, param_1) ((ewram)->gameState.gameFlagsTail[(param_1) >> 5] & (1 << ((param_1) & 0x1F)))

/**
 * @brief (Unused, no pointer in the ROM) Set up the gate graphics; start fully
 * open (unk_4F8 = 0x4000) when the object flag unk_514 is set.
 */
void Gate_CreateFlagControlled(struct EwramData_EntityData *entity)
{
    Gate_SetupGraphics(entity, 0);
    if (OBJECT_FLAG(gEwramData, entity->unk_514.part16.spawnParam0))
        entity->unk_4F8.whole = 0x4000;
}

/**
 * @brief 4DEA0 | Draw; while unk_4F8 (1..0x2F) is set also draw a copy as
 * OBJ window 0x30 px higher and lower the entity by unk_4F8; above 0x2F
 * only switch the OBJ window off.
 */
void Door_Draw(struct EwramData_EntityData *entity)
{
    u8 *framePtr = &entity->animFrame;
    u32 savedFrame = *framePtr;
    u16 *yPtr = &entity->posY.part16.integer;
    u32 savedY = *yPtr;

    if (entity->unk_4F8.whole > 0x2F)
    {
        gUnk_03002CB0.dispCnt &= ~DCNT_WINOBJ;
        return;
    }
    if (entity->unk_4F8.whole != 0)
    {
        entity->objMode = 2;
        gUnk_03002CB0.dispCnt |= DCNT_WINOBJ;
        *yPtr -= 0x30;
        Sprite_Draw(entity);
        entity->objMode = 0;
        *yPtr = savedY - entity->unk_4F8.whole;
    }
    *framePtr = savedFrame;
    Sprite_Draw(entity);
    *yPtr = savedY;
}

/**
 * @brief 4DF3C | Set up the drawing: layer 0x16, facing right, unk_4FE =
 * whether it is in the right half of the screen, animation set
 * sCommonObjectSpriteSheet with palette 6, frame 0x3F, OBJ window on, drawFunc
 * Door_Draw.
 */
void Door_SetupGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx;

    entity->spriteLayer = 0x16;
    entity->flipX = 0;
    entity->unk_4FC.part8.unk_4FE = (s16)entity->posX.part16.integer > 0x78;
    gfx = ObjGfx_Find(sCommonObjectGfx);
    entity->unk_4FC.part8.unk_4FF = 6;
    Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 6);
    entity->animFrame = 0x3F;
    gUnk_03002CB0.dispCnt |= DCNT_WINOBJ;
    entity->drawFunc = (u32 *)Door_Draw;
}
