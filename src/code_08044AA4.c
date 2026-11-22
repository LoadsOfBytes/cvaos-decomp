#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code_08032444.h"
#include "structs/ewram.h"
extern void Effect_UpdateHomingOrb(struct EwramData_EntityData *);
extern u16 Entity_AngleToPoint(struct EwramData_EntityData *, s32, s32);
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];

/**
 * @brief Create a homing visual orb aimed toward the supplied owner and vertical offset with existing random initial angle; preserve the non-NULL owner assumption.
 */
struct EwramData_EntityData *Effect_CreateHomingOrb(struct EwramData_EntityData *owner, s32 x, s32 y, s32 dy, s32 palette)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)Effect_UpdateHomingOrb);
    if (entity != NULL)
    {
        s32 graphics;
        entity->spriteLayer = 3;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, Soul_GetPickupPalette(palette));
        Anim_StartIndexed(entity, 6, 0, 1);
        Anim_Update(entity);
        entity->stage = 1;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->posX.part16.integer = x;
        entity->posY.part16.integer = y;
        entity->scrollWithBg = 1;
        entity->unk_4FC.whole = Entity_AngleToPoint(entity, owner->posX.whole, owner->posY.whole + dy);
        if (owner == gEwramData->playerState.player)
            entity->unk_4FC.whole += 0x6000 + (RandomNumberGenerator() & 0x3FFF);
        else
            entity->unk_4FC.whole += -0x1000 + (RandomNumberGenerator() & 0x1FFF);
        entity->unk_4FC.whole = (u16)entity->unk_4FC.whole;
        entity->unk_500.whole = 0;
        entity->unk_504.whole = 0x30000;
        entity->timer = 200;
        entity->unk_4F8.unk_4F8_entity = owner;
        entity->unk_508.whole = dy;
    }
    return entity;
}
