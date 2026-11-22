#include "code_08009A0.h"
#include "code_08032444.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern void EventScene03SoulOrb_Update(struct EwramData_EntityData *);
extern void EventScene03SoulOrb_OnPlayerContact(struct EwramData_EntityData *);
extern u32 Entity_AngleToPoint(struct EwramData_EntityData *, s32, s32);
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];
/**
 * @brief Spawn a soul orb (soul pickup palette of the variant) that homes in on
 * the player, or waits (mode != 0); its contact callback ends it.
 */
struct EwramData_EntityData *EventScene03_SpawnSoulOrb(s32 x, s32 y, s32 variant, s32 index, s32 mode)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)EventScene03SoulOrb_Update);
    if (entity != NULL)
    {
        s32 graphics;
        entity->spriteLayer = 0x13;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, Soul_GetPickupPalette(variant));
        Anim_StartIndexed(entity, 6, 0, 1);
        Anim_Update(entity);
        entity->stage = 1;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_Enable(entity, 0xB, EventScene03SoulOrb_OnPlayerContact, 0);
        entity->hit.flags |= 4;
        HitBox_SetAttackRaw(entity, 0x0C0CFAFA);
        entity->posX.part16.integer = x;
        entity->posY.part16.integer = y;
        entity->scrollWithBg = 1;
        if (mode == 0)
        {
            struct EwramData_EntityData *player = gEwramData->playerState.player;
            entity->unk_4FC.whole = (Entity_AngleToPoint(entity, player->posX.whole, player->posY.whole - 0x80000) + 0x8000) & 0xFFFF;
            entity->unk_500.whole = mode;
            entity->unk_504.whole = 0x30000;
        }
        else entity->state = 99;
        entity->unk_518.part8.enemyId = variant + 5;
        entity->unk_514.part16.spawnParam1 = index;
        entity->timer = 200;
    }
    return entity;
}
