#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void RockArmorRock_Update(struct EwramData_EntityData *);
extern s32 Entity_GetTargetDirection(struct EwramData_EntityData *);
extern s16 Entity_GetTargetDistance(struct EwramData_EntityData *);

static inline s32 CanAttack(struct EwramData_EntityData *e, s32 range)
{ s32 result; if ((e->flipX && (s8)Entity_GetTargetDirection(e) >= 0) || (!e->flipX && (s8)Entity_GetTargetDirection(e) <= 0)) goto facing;
goto no;
facing: if (Entity_GetTargetDistance(e) > range) goto no;
result = 1; goto end;
no: result = 0;
end: return result; }

void RockArmor_UpdateAttack(struct EwramData_EntityData *entity)
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
            child = Entity_CreateObject((u32 *)RockArmorRock_Update);
            if (child != NULL)
            {
                ENTITY_INDEX(child) = ENTITY_INDEX(entity);
                ENTITY_OWNER(child) = entity;
                child->posX.whole = entity->posX.whole;
                child->posY.whole = entity->posY.whole;
                child->flipX = entity->flipX;
                child->hp = entity->hp;
                child->attack = entity->attack;
            }
        }
        if (entity->animFlags & 4)
        {
            if (CanAttack(entity, 99))
                entity->subState = 0;
            else entity->subState = 2;
        }
        break;
    case 2:
        entity->state = 0;
        entity->stage = 0;
        entity->subState = 0;
        Anim_StartIndexed(entity, 0, 0, 1);
        break;
    }
}
