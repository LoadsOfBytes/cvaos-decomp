/* Waiter Skeleton creation and attached sprite setup. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern void WaiterSkeletonTray_Update(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern void Enemy_OnContact(struct EwramData_EntityData *);
extern void WaiterSkeleton_OnHit(struct EwramData_EntityData *, s32, u16 *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern u8 sWaiterSkeletonGfx[], sSkeletonWaiterSkeletonPalette[], sWaiterSkeletonSpriteSheet[];
void EnemyWaiterSkeletonCreate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *effect;

    if (!Entity_LoadGraphics(entity, sWaiterSkeletonGfx, sSkeletonWaiterSkeletonPalette, 0, sWaiterSkeletonSpriteSheet))
        goto failed;
    entity->ext.waiterSkeleton.flags |= 2;
    effect = Entity_CreateObject(WaiterSkeletonTray_Update);
    if (effect == NULL)
    {
    failed:
        EntityDelete(entity);
        return;
    }
    effect->ext.waiterTray.owner = entity;
    *(u8 *)&effect->ext.raw.unk_4F4 = *(u8 *)&entity->ext.raw.unk_4F4;
    effect->attack = entity->attack;
    *(u16 *)&effect->hp = *(u16 *)&entity->hp;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 19;
    Entity_EnableBoxes(entity, Enemy_OnContact, WaiterSkeleton_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->posY.part16.integer++;
    entity->scrollWithBg = 1;
    entity->state = 0;
}

