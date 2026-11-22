/* Skeleton enemy: graphics, child entity and patrol range around the spawn X. */
#include "code_08009A0.h"
#include "skeleton_entity.h"
extern s32 GetEntityRoomXPositionInteger(struct EwramData_EntityData *);
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern struct EwramData_EntityData *Entity_CreateLinkedAndHide(struct EwramData_EntityData *, u32 *);
extern void SkeletonLinkedPart_Update(struct EwramData_EntityData *);
extern void SkeletonAttachedPart_Update(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern void Entity_EnableBoxes(struct EwramData_EntityData *entity, void *onHit, void *onHurt);
extern void Enemy_OnContact(struct EwramData_EntityData *entity);
extern void Skeleton_OnHit(struct EwramData_EntityData *entity);
extern u8 sSkeletonGfx[], sSkeletonWaiterSkeletonPalette[], sSkeletonSpriteSheet[];

void EnemySkeletonCreate(struct EwramData_EntityData *entity)
{
    void *palette = sSkeletonWaiterSkeletonPalette;
    void *sprite = sSkeletonSpriteSheet;
    u8 *graphics = sSkeletonGfx;
    s32 affine;
    s32 x;
    s32 zero;
    s32 zero2;
    struct EwramData_EntityData *child;

    if (!Entity_LoadGraphics(entity, graphics, palette, 0, sprite))
        goto delete;
    SKELETON_FLAGS(entity) |= 1;
    zero = 0;
    if (Entity_CreateLinkedAndHide(entity, (u32 *)SkeletonLinkedPart_Update) == NULL)
    {
    delete:
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    if (((u8 *)gEwramData)[0x8E] & 0x40)
    {
        entity->state = 5;
        entity->stage = zero;
        entity->subState = zero;
        return;
    }
    affine = ObjAffine_Alloc(1);
    if (affine < 0)
    {
        entity->deleteRequested = 1;
        return;
    }
    zero2 = 0;
    *(u16 *)&entity->unk_504 = affine;
    entity->affineIndex = affine;
    entity->affine = 0;
    entity->spriteLayer = 0x11;
    
    entity->state = zero2;
    entity->stage = zero2;
    entity->subState = zero2;
    Enemy_ResetFromTemplate(entity);
    Entity_EnableBoxes(entity, Enemy_OnContact, Skeleton_OnHit);
    Entity_SnapToGround(entity);
    x = GetEntityRoomXPositionInteger(entity);
    SKELETON_HOME_X(entity) = x;
    SKELETON_PATROL_RIGHT(entity) = x + 0x20;
    SKELETON_PATROL_LEFT(entity) = x - 0x20;
    child = Entity_CreateLinkedAndHide(entity, (u32 *)SkeletonAttachedPart_Update);
    if (child == NULL)
        return;
    child->hidden = 0;
    SKELETON_FLAGS(entity) |= 4;
    if (RandomNumberGenerator() & 1)
        entity->state = 1;
    else
        entity->state = 1;
    entity->stage = zero2;
    entity->subState = zero2;
}
