#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern s32 Entity_GetTargetDirection(struct EwramData_EntityData *);
extern s16 Entity_GetTargetDistance(struct EwramData_EntityData *);
extern u8 sBomberRockArmorWalkProfile[];
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void BomberArmorBomb_Update(struct EwramData_EntityData *);
static inline s32 CanAttack(struct EwramData_EntityData *e, s32 range)
{
    s32 result;
    if ((e->flipX && (s8)Entity_GetTargetDirection(e) >= 0) || (!e->flipX && (s8)Entity_GetTargetDirection(e) <= 0))
        goto facing;
    goto no;
facing:
    if (Entity_GetTargetDistance(e) > range) goto no;
    result = 1;
    goto end;
no:
    result = 0;
end:
    return result;
}

static inline s32 Speed(s32 phase, s32 base) { return (phase << 14) + base; }
void BomberRockArmor_UpdateWalk(struct EwramData_EntityData *entity)
{
    s32 state = entity->subState;
    if (state == 0)
    {
        u8 phase;
        s32 speed;
        s32 collision;
        if (*(u16 *)&entity->animStep == 9 && entity->unk_514.part16.spawnParam0 != 0)
        {
            if (entity->timer++ > entity->unk_514.part16.spawnParam0)
            {
                entity->timer = state;
                entity->flipX ^= 1;
            }
        }
        phase = (u32)entity->animStep % 5;
        speed = Speed(phase, 0x2000);
        if (phase > 3) speed = 0;
        collision = Entity_Walk(entity, speed, sBomberRockArmorWalkProfile, 0x14);
        if (entity->unk_514.part16.spawnParam0 == 0 && (collision & 0x1B) != 8)
            entity->flipX ^= 1;
        if (entity->animTimer != 0) return;
        if (entity->animStep == 9 && CanAttack(entity, 99))
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
    }
}

/**
 * @brief Attack state machine (state 1): animation 1 and at step 7 throw a
 * bouncing bomb (BomberArmorBomb_Update, sound 0x88); back to state 0 when
 * the animation ends.
 */
void BomberArmor_UpdateAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->subState = 1;
        break;
    case 1:
        if (*(u16 *)&entity->animStep == 7)
        {
            struct EwramData_EntityData *child;
            PlaySong(0x88);
            child = Entity_CreateObject((u32 *)BomberArmorBomb_Update);
            if (child != NULL)
            {
                child->unk_4F4.part8.unk_4F4 = entity->unk_4F4.part8.unk_4F4;
                child->unk_4F8.unk_4F8_entity = entity;
                child->posX.whole = entity->posX.whole;
                child->posY.whole = entity->posY.whole;
                child->flipX = entity->flipX;
                child->unk_518.part16.hp = entity->unk_518.part16.hp;
                child->attack = entity->attack;
            }
        }
        if (entity->animFlags & 4)
        {
            if (CanAttack(entity, 99))
            {
                Anim_StartIndexed(entity, 0, 0, 0);
                entity->subState = 2;
            }
            else entity->subState = 3;
        }
        break;
    case 2:
        {
            u8 phase = (u32)entity->animStep % 5;
            s32 speed = Speed(phase, 0x2000);
            if (phase > 3) speed = 0;
            Entity_Walk(entity, -speed, sBomberRockArmorWalkProfile, 0x14);
            if (entity->animFlags & 4) entity->subState = 0;
        }
        break;
    case 3:
        entity->state = 0;
        entity->stage = 0;
        entity->subState = 0;
        Anim_StartIndexed(entity, 0, 0, 1);
        break;
    }
}
