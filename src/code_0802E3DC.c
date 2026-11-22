#include "code_08009A0.h"
#include "blue_soul_entity.h"
#include "code_080D73B8.h"
#include "code/code_08040A38.h"
extern s32 BlueSoul_ShouldEnd(struct EwramData_EntityData *);

static inline void FollowParent(struct EwramData_EntityData *entity, struct EwramData_EntityData *parent, u16 x_, u16 y_)
{
    s16 x = x_, y = y_;
    entity->flipX = parent->flipX;
    if (entity->flipX) x = -x;
    entity->posX.part16.integer = (s32)x + (s16)parent->posX.part16.integer;
    entity->posY.part16.integer = (s32)y + parent->posY.part16.integer;
}

void SkillBlackPantherUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = BLUE_SOUL_OWNER(entity);


    if (BlueSoul_ShouldEnd(entity))
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        entity->scrollWithBg = 1;
        Hit_Enable(entity, 6, 0, 0);
        HitBox_SetAttackRaw(entity, 0x3030D8F4);
        entity->hit.flags |= 4;
        Hit_SetTimerOverride(2, 0x3C);
        {
            u16 a = 8;
            u16 b = 1;
            gEwramData->playerState.skillBaseAttack = a;
            gEwramData->playerState.skillAttackAttributes = b;
        }
        entity->state++;
    case 1:
        if (parent->animScriptIndex == 3)
        {
            if (entity->subState == 0)
            {
                entity->subState = 1;
                PlaySong(0x9A);
            }
            entity->hit.flags &= ~2;
        }
        else
        {
            entity->subState = 0;
            entity->hit.flags |= 2;
        }
        FollowParent(entity, parent, 0, 0);
    }
    gEwramData->playerStatus.soulStateFlags |= BLUE_SOUL_VALUE(entity);
}

