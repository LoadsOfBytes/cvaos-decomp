#include "code_08009A0.h"
#include "soul_skill_entity.h"
#include "player_entity.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern s32 BlueSoul_CheckEndCondition(struct EwramData_EntityData *entity);
extern void BlueSoul_ReleaseActiveResources(struct EwramData_EntityData *entity);
extern void SkillBuerOrbitUpdate(struct EwramData_EntityData *entity);

#define SIZE(e) (*(s32 *)&(e)->unk_514)

/**
 * @brief 2FF40 | Buer blue soul (ends via BlueSoul_CheckEndCondition): Hit_SetTimerOverride(2,
 * 0x18), skillBaseAttack/skillAttackAttributes = 12 / 2, two orbiting children SkillBuerOrbitUpdate (angles
 * 0 and 0x8000, speed 0x200 against the caster's facing), songs 0x1A5 and
 * 0x92; follows the caster with a vertical offset (velY) moving towards
 * -20 px, or -8 px while the caster has unk_4F4 bit 10, while unk_514
 * counts up to 28; on ending unk_514 counts down by 2 per frame, then
 * BlueSoul_ReleaseActiveResources and delete.
 */
void SkillBuerUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    struct EwramData_EntityData *child;
    s32 speed;
    u8 state;

    entity->roomActive = 1;
    {
        u8 two = 2;
        if (entity->state < two && BlueSoul_CheckEndCondition(entity))
            entity->state = two;
    }
    state = entity->state;
    switch (state)
    {
    case 0:
        Hit_SetTimerOverride(2, 0x18);
        {
            u16 a = 0xC;
            u16 b = 2;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        entity->scrollWithBg = 1;
        entity->velY.whole = -0x100000;
        speed = 0x200;
        if (parent->flipX)
            speed = -speed;
        child = BlueSoul_CreateEntity(entity, SkillBuerOrbitUpdate);
        if (child != NULL)
        {
            SKILL_BUER_ANGLE(child) = state;
            SKILL_BUER_SPEED(child) = speed;
        }
        child = BlueSoul_CreateEntity(entity, SkillBuerOrbitUpdate);
        if (child != NULL)
        {
            SKILL_BUER_ANGLE(child) = 0x8000;
            SKILL_BUER_SPEED(child) = speed;
        }
        PlaySong(0x1A5);
        PlaySong(0x92);
        entity->state++;
    case 1:
        if (SIZE(entity) <= 0x1B)
            SIZE(entity)++;
        entity->posX.part16.integer = parent->posX.part16.integer;
        entity->posY.part16.integer = parent->posY.part16.integer + entity->velY.part16.integer;
        if (parent->ext.player.actionFlags & 0x400)
        {
            if (entity->velY.whole < -0x80000)
                entity->velY.whole += 0x4000;
        }
        else if (entity->velY.whole > -0x140000)
            entity->velY.whole -= 0x4000;
        break;
    case 2:
        if (SIZE(entity) > 0)
            SIZE(entity) -= 2;
        else
        {
            BlueSoul_ReleaseActiveResources(entity);
            EntityDelete(entity);
        }
        break;
    }
}
