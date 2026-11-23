#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code_08032444.h"
#include "structs/ewram.h"

extern void SoulPickup_Update(struct EwramData_EntityData *);
extern void Pickup_Collect(struct EwramData_EntityData *);
extern u16 Entity_AngleToPoint(struct EwramData_EntityData *, s32, s32);
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];

/**
 * @brief Create an animated soul pickup with soul kind/id and collection callback; select homing initialization or state 0x63 from the supplied variant.
 */
struct EwramData_EntityData *SoulPickup_Create(s32 x, s32 y, s32 palette, s32 soulId, s32 stationary)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)SoulPickup_Update);
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
        Hit_Enable(entity, 0xB, Pickup_Collect, 0);
        entity->hit.flags |= 4;
        HitBox_SetAttackRaw(entity, 0x1010F8F8);
        entity->posX.part16.integer = x;
        entity->posY.part16.integer = y;
        entity->scrollWithBg = 1;
        if (stationary == 0)
        {
            struct EwramData_EntityData *player = gEwramData->playerState.player;
            s32 px = player->posX.whole;
            s32 py = player->posY.whole;
            py -= (RandomNumberGenerator() & 0x7F) << 12;
            py += -0x80000;
            entity->ext.soulPickup.angle = (Entity_AngleToPoint(entity, px, py) + 0x8000) & 0xFFFF;
            entity->ext.soulPickup.turn = stationary;
            entity->ext.soulPickup.speed = 0x30000;
        }
        else
        {
            entity->state = 0x63;
        }
        entity->enemyId = palette + 5;
        entity->spawnParam1 = soulId;
        entity->timer = 200;
    }
    return entity;
}
