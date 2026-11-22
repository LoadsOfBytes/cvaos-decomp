#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code_08032444.h"
#include "structs/ewram.h"

/*
 * Homing orb (Effect_CreateHomingOrb / Effect_UpdateHomingOrb): the entity it
 * flies to and the y offset of the aim point, its flight angle, the turn rate
 * per frame (grows to 0x4000) and the 16.16 speed (grows to 0x80000).
 * Token-identical expansion.
 */
#define HOMING_ORB_TARGET(e) e->unk_4F8.unk_4F8_entity
#define HOMING_ORB_TARGET_DY(e) e->unk_508.whole
#define HOMING_ORB_ANGLE(e) e->unk_4FC.whole
#define HOMING_ORB_TURN(e) e->unk_500.whole
#define HOMING_ORB_SPEED(e) e->unk_504.whole
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
        HOMING_ORB_ANGLE(entity) = Entity_AngleToPoint(entity, owner->posX.whole, owner->posY.whole + dy);
        if (owner == gEwramData->playerState.player)
            HOMING_ORB_ANGLE(entity) += 0x6000 + (RandomNumberGenerator() & 0x3FFF);
        else
            HOMING_ORB_ANGLE(entity) += -0x1000 + (RandomNumberGenerator() & 0x1FFF);
        HOMING_ORB_ANGLE(entity) = (u16)HOMING_ORB_ANGLE(entity);
        HOMING_ORB_TURN(entity) = 0;
        HOMING_ORB_SPEED(entity) = 0x30000;
        entity->timer = 200;
        HOMING_ORB_TARGET(entity) = owner;
        HOMING_ORB_TARGET_DY(entity) = dy;
    }
    return entity;
}
