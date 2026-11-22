/* Waiter Skeleton: choose and create a thrown object, then resume its walk. */
#include "code_08009A0.h"
#include "waiter_skeleton_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern void WaiterSkeletonThrownTray_Update(struct EwramData_EntityData *);
void WaiterSkeleton_UpdateThrow(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        if ((WAITER_SKELETON_FLAGS(entity) & 2) && RandomNumberGenerator() % 4 == 0)
            entity->subState = 1;
        else
            entity->subState = 3;
        break;
    case 1:
        {
            struct EwramData_EntityData *child = Entity_CreateObject(WaiterSkeletonThrownTray_Update);
            if (child != NULL)
            {
                WAITER_TRAY_OWNER(child) = entity;
                *(u8 *)&child->unk_4F4 = *(u8 *)&entity->unk_4F4;
                child->attack = entity->attack;
                child->unk_518.part16.hp = entity->unk_518.part16.hp;
                WAITER_TRAY_ANIM(child) = 17;
            }
            Anim_StartIndexed(entity, 6, 0, 0);
            entity->subState = 2;
        }
        break;
    case 2:
        WAITER_SKELETON_FLAGS(entity) &= ~2;
        goto finishThrow;
    case 3:
        {
            struct EwramData_EntityData *child = Entity_CreateObject(WaiterSkeletonThrownTray_Update);
            if (child != NULL)
            {
                WAITER_TRAY_OWNER(child) = entity;
                *(u8 *)&child->unk_4F4 = *(u8 *)&entity->unk_4F4;
                child->attack = entity->attack;
                child->unk_518.part16.hp = entity->unk_518.part16.hp;
                WAITER_SKELETON_FLAGS(entity) |= 1;
                WAITER_TRAY_ANIM(child) = 9;
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
