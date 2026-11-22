#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void KickerSkeleton_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void KickerSkeleton_OnHit(struct EwramData_EntityData *, s32, u16 *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern u8 sKickerSkeletonGfx[], sKickerSkeletonPalette[], sKickerSkeletonSpriteSheet[];

void EnemyKickerSkeletonCreate(struct EwramData_EntityData *entity)
{
    if (!Entity_LoadGraphics(entity, sKickerSkeletonGfx, sKickerSkeletonPalette, 0, sKickerSkeletonSpriteSheet))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x13;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview)
        entity->state = 3;
    else
    {
        Entity_EnableBoxes(entity, KickerSkeleton_DamagePlayerOnContact, KickerSkeleton_OnHit);
        Entity_UpdateBoxes(entity);
        Entity_SnapToGround(entity);
        entity->posY.part16.integer -= 0x10;
        entity->scrollWithBg = 1;
        entity->state = 0;
    }
}
