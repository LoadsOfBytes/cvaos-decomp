/* Attack step of a part-driven enemy (Nemesis_UpdateAttackSelection, cf. Alastor Alastor_UpdateAttackSelection):
 * rotate the part through three target angles, then pick the next attack. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 Angle_TurnTowards(s16 *, u16, u16);
extern s32 sUnk_08528A34[][9];

#define SET(e) (e->ext.nemesis.attackSet)
#define INDEX(e) (e->ext.nemesis.attackIndex)
#define ATTACK(e) (sUnk_08528A34[SET(e)][INDEX(e)])
#define ANGLE(e) (e->ext.nemesis.angle)
#define STEP(e) (e->ext.nemesis.turn)
#define ABS(x) ((x) < 0 ? -(x) : (x))

static inline void SetState(struct EwramData_EntityData *entity, u8 state)
{
    if (entity->state != state)
    {
        entity->state = state;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief Read the randomized attack sequence, coordinate partner state 2 with angular transitions and dispatch the selected attack state.
 */
void Nemesis_UpdateAttackSelection(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[entity->ext.nemesis.partnerIndex];
    s32 speed = 0x20000;
    s32 target = 0xD800;

    if (entity->subState == 0)
    {
        if (INDEX(entity) == 9)
        {
            SET(entity) = RandomNumberGenerator() % 4;
            INDEX(entity) = 0;
        }
        if (ATTACK(entity) == 2)
        {
            SetState(entity, 5);
            INDEX(entity)++;
        }
    }
    switch (entity->subState)
    {
        case 0:
            if (!(entity->ext.nemesis.flags & 2))
            {
                speed = -speed;
                target = 0x10000 - target;
            }
            entity->posX.whole += speed;
            if (Angle_TurnTowards(&ANGLE(entity), (u16)ABS(STEP(entity) << 3), (u16)target))
            {
                SetState(part, 2);
                entity->subState++;
            }
            goto transform;
        case 1:
            if (part->state == 0)
                entity->subState++;
            break;
        case 2:
            target = 0x9800;
            if (!(entity->ext.nemesis.flags & 2))
                target = 0x6800;
            if (Angle_TurnTowards(&ANGLE(entity), (u16)ABS(STEP(entity) << 4), (u16)target))
                entity->subState++;
            goto transform;
        case 3:
            target = 0x2800;
            if (!(entity->ext.nemesis.flags & 2))
                target = 0xD800;
            if (Angle_TurnTowards(&ANGLE(entity), (u16)ABS(STEP(entity) << 4), (u16)target))
                entity->subState++;
transform:
            ObjAffine_Set(ANGLE(entity), 0x10000, 0x10000, 0, 0, entity->affineIndex);
            break;
        case 4:
        {
            s32 state = part->state;
            if (state != 0)
                break;
            if (INDEX(entity) == 9)
            {
                SET(entity) = RandomNumberGenerator() % 4;
                INDEX(entity) = state;
            }
            switch (ATTACK(entity))
            {
                case 0: SetState(entity, 3); break;
                case 1: SetState(entity, 4); break;
                case 2: SetState(entity, 5); break;
            }
            INDEX(entity)++;
            break;
        }
    }
}
