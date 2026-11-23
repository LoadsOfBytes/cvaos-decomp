/* SkeletonProjectile_Update: debris piece thrown by its parent entity (falls until off screen). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *entity);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void SkeletonProjectile_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void SkeletonProjectile_OnHit(struct EwramData_EntityData *);
extern u8 sSkeletonSpriteSheet[];


static inline s32 IsBelow(struct EwramData_EntityData *entity)
{
    if (entity->posY.part16.integer > 0xAA)
        return 1;
    return 0;
}

/**
 * @brief Attach to the parent graphics and affine slot, launch and fall; decrement the parent projectile count before deletion.
 */
void SkeletonProjectile_Update(struct EwramData_EntityData *entity)
{
    u32 box[4];
    struct EwramData_EntityData *parent = &gEwramData->entityData[(u8)entity->ext.skeletonPiece.ownerIndex];
    u32 state;
    u8 flag;

    switch (entity->state)
    {
    case 0:
        entity->scrollWithBg = 1;
        entity->basePalette = parent->basePalette;
        Sprite_Attach(entity, sSkeletonSpriteSheet, state = Sprite_GetTileSlot(parent), entity->basePalette);
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->hp = 1;
        entity->mp = 1;
        entity->affineIndex = parent->ext.skeleton.affine;
        Hit_Enable(entity, 0xA, SkeletonProjectile_DamagePlayerOnContact, (s32)SkeletonProjectile_OnHit);
        HitBox_SetBodyRaw(entity, 0x1010F8F8);
        HitBox_SetAttackRaw(entity, 0x0202FFFF);
        entity->attack = parent->attack;
        entity->state++;
    case 1:
        entity->affine = 1;
        if (parent->flipX)
        {
            entity->velX.whole = 0xD000;
            entity->accelX.whole = -0x1000;
        }
        else
        {
            entity->velX.whole = -0xD000;
            entity->accelX.whole = 0x1000;
        }
        entity->posX.part16.integer = parent->posX.part16.integer;
        entity->posY.part16.integer = parent->posY.part16.integer - 0x1E;
        entity->velY.whole = -0x30000;
        entity->scrollWithBg = 1;
        if (parent->flipX)
            entity->velX.whole = 0x8AAA;
        else
            entity->velX.whole = -0x8AAA;
        if ((u8)entity->ext.skeletonBone.kind == 0)
        {
            PlaySong(0x88);
            entity->animFrame = 0x12;
        }
        else
        {
            PlaySong(0x89);
            entity->animFrame = 0x1B;
        }
        entity->state = 2;
        break;
    case 2:
    case 3:
        if (entity->state != 4)
            Enemy_UpdateHitFlash(entity);
        if (entity->state != 3)
        {
            if (Entity_IsTimeStopped(entity))
            {
                Hit_CheckWithPlayer(box, entity);
                entity->affine = 0;
                break;
            }
            entity->affine = 1;
        }
        flag = entity->hitFlashTimer;
        state = entity->state;
        if (flag && (u8)state != 3 && (u8)state != 4)
        {
            Hit_CheckWithPlayer(box, entity);
            break;
        }
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x1800;
        if ((u8)state != 3)
            Hit_CheckWithPlayer(box, entity);
        if (IsBelow(entity))
            entity->state = 4;
        break;
    case 4:
        parent->ext.skeleton.throwCount--;
        entity->deleteRequested = 1;
        break;
    }
}
