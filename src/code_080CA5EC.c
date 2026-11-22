/* Beam Skeleton creation and update dispatcher. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity,u8 *graphics,void *palette,u8 slot,void *sprite);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern void Entity_EnableBoxes(struct EwramData_EntityData *entity,void *callback,s32 hitCallback);
extern void Enemy_OnContact(struct EwramData_EntityData *entity);
extern void BeamSkeleton_OnHit(struct EwramData_EntityData *entity);
extern s32 GetEntityRoomXPositionInteger(struct EwramData_EntityData *entity);
extern void BeamSkeleton_UpdateBestiaryPreview(struct EwramData_EntityData *entity);
extern void BeamSkeleton_UpdateDeath(struct EwramData_EntityData *entity);
extern void BeamSkeleton_UpdateFire(struct EwramData_EntityData *entity);
extern void BeamSkeleton_UpdatePatrol(struct EwramData_EntityData *entity);
extern void BeamSkeleton_UpdateApproach(struct EwramData_EntityData *entity);
extern void BeamSkeleton_UpdateIdle(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *entity);
extern u8 sBeamSkeletonPalette[];
extern u8 sBeamSkeletonSpriteSheet[];
extern u8 sBeamSkeletonGfx[];

static inline void SetAffine(struct EwramData_EntityData *e,s32 index) { e->affineIndex = index; }
void EnemyBeamSkeletonCreate(struct EwramData_EntityData *entity)
{
    void *palette = sBeamSkeletonPalette;
    void *sprite = sBeamSkeletonSpriteSheet;
    u8 *graphics = sBeamSkeletonGfx;
    u8 display;
    u16 affine;
    s32 x;
    if (!Entity_LoadGraphics(entity,graphics,palette,0,sprite))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity,0,0,1);
    Anim_Update(entity);
    /* Read the whole flag byte: this caller retains mask 0x40 in display. */
    display = ((u8 *)&gEwramData->gameState)[0x2E] & 0x40;
    if (display != 0)
    {
        entity->state = 4;
        entity->stage = 0;
        entity->subState = 0;
        return;
    }
    affine = ObjAffine_Alloc(1);
    if ((s16)affine < 0)
    {
        EntityDelete(entity);
        return;
    }
    SetAffine(entity,affine);
    entity->affine = 0;
    ObjAffine_Set(0,0x10000,0x10000,entity->flipX,display,entity->affineIndex);
    entity->spriteLayer = 17;
    Enemy_ResetFromTemplate(entity);
    entity->state = display;
    entity->stage = display;
    entity->subState = display;
    Entity_EnableBoxes(entity,Enemy_OnContact,(s32)BeamSkeleton_OnHit);
    Entity_SnapToGround(entity);
    x = GetEntityRoomXPositionInteger(entity);
    entity->unk_504.part16.unk_506[0] = x;
    entity->unk_4F8.part16.unk_4F8 = x + 64;
    entity->unk_4F8.part16.unk_4FA = x - 64;
}

void EnemyBeamSkeletonUpdate(struct EwramData_EntityData *entity)
{
    if (entity->state == 4)
        BeamSkeleton_UpdateBestiaryPreview(entity);
    else if (entity->state == 0xFF)
        BeamSkeleton_UpdateDeath(entity);
    else if (entity->state == 3)
    {
        BeamSkeleton_UpdateFire(entity);
        Entity_AnimateWithBoxes(entity);
        Enemy_CheckHitPlayer(entity);
    }
    else if (!Enemy_PreUpdate(entity))
    {
        entity->unk_500.part16.unk_502 = RandomNumberGenerator() % 80 + 40;
        switch (entity->state)
        {
            case 0: BeamSkeleton_UpdatePatrol(entity); break;
            case 1: BeamSkeleton_UpdateApproach(entity); break;
            case 2: BeamSkeleton_UpdateIdle(entity); break;
        }
        Entity_AnimateWithBoxes(entity);
        Enemy_CheckHitPlayer(entity);
    }
}
