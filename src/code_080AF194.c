/* Waiter Skeleton: choose and create a thrown object, then resume its walk. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern void WaiterSkeletonThrownTray_Update(struct EwramData_EntityData *);
void WaiterSkeleton_UpdateThrow(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        if ((entity->ext.waiterSkeleton.flags & 2) && RandomNumberGenerator() % 4 == 0)
            entity->subState = 1;
        else
            entity->subState = 3;
        break;
    case 1:
        {
            struct EwramData_EntityData *child = Entity_CreateObject(WaiterSkeletonThrownTray_Update);
            if (child != NULL)
            {
                child->ext.waiterTray.owner = entity;
                *(u8 *)&child->ext.raw.unk_4F4 = *(u8 *)&entity->ext.raw.unk_4F4;
                child->attack = entity->attack;
                child->hp = entity->hp;
                child->ext.waiterTray.anim = 17;
            }
            Anim_StartIndexed(entity, 6, 0, 0);
            entity->subState = 2;
        }
        break;
    case 2:
        entity->ext.waiterSkeleton.flags &= ~2;
        goto finishThrow;
    case 3:
        {
            struct EwramData_EntityData *child = Entity_CreateObject(WaiterSkeletonThrownTray_Update);
            if (child != NULL)
            {
                child->ext.waiterTray.owner = entity;
                *(u8 *)&child->ext.raw.unk_4F4 = *(u8 *)&entity->ext.raw.unk_4F4;
                child->attack = entity->attack;
                child->hp = entity->hp;
                entity->ext.waiterSkeleton.flags |= 1;
                child->ext.waiterTray.anim = 9;
            }
            Anim_StartIndexed(entity, 7, 0, 0);
            entity->subState = 4;
        }
        break;
    case 4:
finishThrow:
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 3, 0, 1);
            entity->state = 2;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}
