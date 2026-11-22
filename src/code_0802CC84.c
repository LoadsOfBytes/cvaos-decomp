#include "code_08009A0.h"
#include "red_soul_entity.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08040A38.h"
extern void SkillHarpyProjectileUpdate(struct EwramData_EntityData *);

void SkillHarpyUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = RED_SOUL_OWNER(entity);
    s32 i;
    switch (entity->state)
    {
    case 0:
        Hit_SetTimerOverride(1, 20);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        entity->state = 1;
        Entity_PlaceRelativeTo(entity, parent, 20, -26);
        PlaySong(0xE6);
    case 1:
        for (i = 0; i <= 1; i++)
        {
            struct EwramData_EntityData *child = RedSoul_CreateEntity(entity, (u32 *)SkillHarpyProjectileUpdate);
            if (child == NULL) break;
            child->posX.whole = entity->posX.whole;
            child->posY.whole = entity->posY.whole;
            child->velX.whole = 0x50000;
            child->flipX = i;
        }
        entity->state = 2;
        break;
    case 2:
        gEwramData->playerState.activeRedSoulEntities[RED_SOUL_SLOT(entity)] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        return;
    }
    entity->roomActive = 1;
}
