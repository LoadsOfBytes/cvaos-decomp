/* AxeArmor_UpdateAttack: Axe Armor attack substates (throws the axe AxeArmorAxe_Update); matched. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Entity_Walk(struct EwramData_EntityData *entity, s32 speed, void *profile, s32 flags);
extern s32 Entity_GetTargetDirection(struct EwramData_EntityData *entity);
extern s16 Entity_GetTargetDistance(struct EwramData_EntityData *entity);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *updateFunc);
extern void AxeArmorAxe_Update(struct EwramData_EntityData *entity);
extern u8 sAxeArmorWalkProfile[];

/**
 * @brief Choose a throw animation, create an axe on its event, then retreat or resume walking.
 */
void AxeArmor_UpdateAttack(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *axe;
    u16 dx;
    s32 distance;
    s32 m1;
    u8 frame;
    s32 speed;
    s32 attack;

    m1 = -1;
    switch (entity->subState)
    {
    case 0:
        if (RandomNumberGenerator() & 1)
        {
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->stage = 0;
        }
        else
        {
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->stage = 1;
        }
        entity->subState = 1;
        break;
    case 1:
        if (*(u16 *)&entity->animStep == 0x20C)
        {
            axe = Entity_CreateObject((u32 *)AxeArmorAxe_Update);
            if (axe != NULL)
            {
                dx = -0x18;
                axe->posX.part16.integer = entity->posX.part16.integer;
                axe->posY.part16.integer = entity->posY.part16.integer - 0x10;
                axe->flipX = entity->flipX;
                if (axe->flipX)
                    dx = 0x18;
                {
                    s32 t = (s16)dx;
                    axe->posX.part16.integer += t;
                }
                axe->attack = entity->attack;
                axe->stage = entity->stage;
            }
        }
        if (!(entity->animFlags & 4))
            break;
        if ((entity->flipX && (s8)Entity_GetTargetDirection(entity) == 1)
            || (!entity->flipX && (s8)Entity_GetTargetDirection(entity) == m1))
        {
            distance = Entity_GetTargetDistance(entity);
            attack = 1;
            if (distance <= 99)
                goto test;
        }
        attack = 0;
    test:
        if (attack)
        {
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->subState = 2;
        }
        else
            entity->subState = 3;
        break;
    case 2:
        frame = entity->animStep % 9;
        speed = frame * 0x2800 + 0x1000;
        if (frame > 6)
            speed = 0;
        Entity_Walk(entity, -speed, sAxeArmorWalkProfile, 0x14);
        if (entity->animFlags & 4)
            entity->subState = 0;
        break;
    case 3:
        entity->state = 0;
        entity->stage = 0;
        entity->subState = 0;
        Anim_StartIndexed(entity, 0, 0, 1);
        break;
    }
}
