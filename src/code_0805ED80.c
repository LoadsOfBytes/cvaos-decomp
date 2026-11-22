#include "code_08009A0.h"
#include "event_scene03.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern void EventScene03TargetOrb_Update(struct EwramData_EntityData *);
extern void EventScene03TargetOrb_OnContact(struct EwramData_EntityData *);
extern u32 Entity_AngleToPoint(struct EwramData_EntityData *, s32, s32);
extern u8 sCommonObjectGfx[], sCommonObjectSpriteSheet[];

/**
 * @brief Spawn an orb that homes in on the given target entity (deleted on
 * arrival with sound 0xBC).
 */
struct EwramData_EntityData *EventScene03_SpawnTargetOrb(s32 x, s32 y, s32 variant, s32 index, struct EwramData_EntityData *target)
{
    struct EwramData_EntityData *entity = EntityCreateInRange(0x49, 0xCF, (u32 *)EventScene03TargetOrb_Update);
    if (entity != NULL)
    {
        s32 graphics, targetX, offset;
        EVENT03_ORB_TARGET(entity) = target;
        entity->spriteLayer = 0x13;
        graphics = ObjGfx_Find(sCommonObjectGfx);
        Sprite_Attach(entity, sCommonObjectSpriteSheet, graphics, 6);
        Anim_StartIndexed(entity, 0x16, 0, 1);
        Anim_Update(entity);
        entity->stage = 1;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        Hit_Enable(entity, 0xB, EventScene03TargetOrb_OnContact, 0);
        entity->hit.flags |= 4;
        HitBox_SetAttackRaw(entity, 0x0C0CFAFA);
        entity->posX.part16.integer = x;
        entity->posY.part16.integer = y;
        entity->scrollWithBg = 1;
        targetX = target->posX.whole;
        offset = ((RandomNumberGenerator() & 0x7F) << 12) + 0x80000;
        EVENT03_ORB_HEADING(entity) = (Entity_AngleToPoint(entity, targetX, target->posY.whole - offset) + 0x8000) & 0xFFFF;
        EVENT03_ORB_TURN_RATE(entity) = 0;
        EVENT03_ORB_SPEED(entity) = 0x30000;
        entity->unk_518.part8.enemyId = variant + 5;
        entity->unk_514.part16.spawnParam1 = index;
        entity->timer = 200;
    }
    return entity;
}
