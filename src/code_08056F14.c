/* Object 0x22 helpers 0x08056F14..0x08057003. */
#include "code_08009A0.h"
#include "room_object_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08050A3C.h"
#include "structs/ewram.h"

extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern void Player_QueueEntity(struct EwramData_EntityData *entity, u8 collisionFlags);
extern void Object22_UpdateState(struct EwramData_EntityData *e);
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
void Object22Effect_Update(struct EwramData_EntityData *entity);

extern u8 sObject22Gfx[];
extern u8 sUnk_08209FD8[];
extern u8 sObject22SpriteSheet[];

/**
 * @brief 56F14 | Object 0x22 update.
 */
void Object22Update(struct EwramData_EntityData *entity)
{
    u8 collision[0x10];

    if (!(entity->ext.object22.flags & 1))
        Player_QueueEntity(entity, 1);
    if (entity->state == 0)
        Object22_UpdateState(entity);
    Anim_Update(entity);
    Hit_CheckWithPlayer(collision, entity);
}

/**
 * @brief 56F50 | Spawn a short-lived effect (animation 2 of sObject22SpriteSheet)
 * at (dx, dy) lasting param_3 frames.
 */
void Object22_SpawnEffect(struct EwramData_EntityData *entity, s32 dx, s32 dy, u8 duration)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)Object22Effect_Update);
    s32 gfx;

    if (child != NULL)
    {
        child->posX.whole = dx;
        child->posY.whole = dy;
        child->spriteLayer = entity->spriteLayer;
        gfx = ObjGfx_Find(sObject22Gfx);
        child->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sUnk_08209FD8, 1);
        Sprite_Attach(child, sObject22SpriteSheet, gfx, child->unk_510.part8.basePalette);
        Anim_StartIndexed(child, 2, 0, 0);
        Anim_Update(child);
        child->timer = duration;
        child->scrollWithBg = 1;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/**
 * @brief 56FE8 | Effect update: delete when the timer timer runs out.
 */
void Object22Effect_Update(struct EwramData_EntityData *entity)
{
    if ((s8)--entity->timer <= 0)
        EntityDelete(entity);
}
