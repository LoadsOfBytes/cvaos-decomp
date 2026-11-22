/* Man Eater projectile and effect controllers. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "syscalls.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern s32 Entity_GetTargetAngleAbove16(struct EwramData_EntityData *);
extern s32 Target_GetPixelX(struct EwramData_EntityData *);
extern s32 Target_GetPixelY(struct EwramData_EntityData *);
extern void ManEaterProjectile_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void ManEaterProjectile_OnHit(struct EwramData_EntityData *);
extern u8 sManEaterGfx[], sManEaterPalette[], sManEaterSpriteSheet[];

/**
 * @brief Display frame 0x19 for a countdown, enabling drawSkip halfway
 * through, then remove the burst effect.
 */
void ManEaterProjectileBurst_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sManEaterGfx, sManEaterPalette, 0, sManEaterSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        entity->animFrame = 0x19;
        entity->spriteLayer = 0x11;
        entity->timer = 0x10;
        entity->state = 1;
        /* fall through */
    case 1:
        if (entity->timer != 0)
        {
            if (entity->timer == 8) entity->drawSkip = 1;
            entity->timer--;
        }
        else EntityDelete(entity);
        break;
    }
}

/**
 * @brief Aim a destructible projectile toward the target and accelerate it.
 * Keep hit checks during time stop; spawn the burst when HP is cleared
 * and remove on the following state update.
 */
void ManEaterProjectile_Update(struct EwramData_EntityData *entity)
{
    u32 box[4];
    s32 angle;
    struct EwramData_EntityData *effect;
    if (entity->state != 0 && (s16)entity->unk_518.part16.hp != 0 && Entity_IsTimeStopped(entity))
    {
        Hit_CheckWithPlayer(box, entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sManEaterGfx, sManEaterPalette, 0, sManEaterSpriteSheet)) goto remove;
        Anim_StartIndexed(entity, 5, 0, 0);
        entity->spriteLayer = 0x11;
        entity->scrollWithBg = 1;
        entity->unk_518.part16.hp = 1;
        Hit_Enable(entity, 0xA, ManEaterProjectile_DamagePlayerOnContact, (s32)ManEaterProjectile_OnHit);
        HitBox_SetBodyRaw(entity, 0x0C0CFAFA);
        HitBox_SetAttackRaw(entity, 0x0C0CFAFA);
        angle = (s16)Entity_GetTargetAngleAbove16(entity);
        entity->velX.whole = (Sin(angle + 0x4000) >> 8) << 6;
        entity->velY.whole = (Sin(angle) >> 8) << 6;
        entity->accelX.whole = entity->velX.whole / 32;
        entity->accelY.whole = entity->velY.whole / 32;
        entity->state = 1;
        /* fall through */
    case 1:
        EntityAccelerateAndMove(entity, 0x10000);
        Hit_CheckWithPlayer(box, entity);
        if (EntityIsOutsideScreen(entity, 3)) entity->state = 2;
        if ((s16)entity->unk_518.part16.hp == 0)
        {
            effect = EntityCreateInRange(0x49, 0xCF, (u32 *)ManEaterProjectileBurst_Update);
            if (effect != NULL)
            {
                effect->posX.whole = entity->posX.whole;
                effect->posY.whole = entity->posY.whole;
                effect->scrollWithBg = 1;
            }
            entity->state = 2;
        }
        break;
    case 2:
remove:
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
}

/**
 * @brief Follow the terminal chain segment, then move toward the target
 * while the linked head attack flag is set. Remove during head breakup.
 */
void ManEaterChainAnchor_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[(u8)entity->unk_4F8.part8.unk_4F8];
    struct EwramData_EntityData *target = &gEwramData->entityData[entity->unk_4F8.part8.unk_4F9];
    s32 dx;
    s32 dy;
    u16 angle;
    if ((parent->unk_4F4.part8.unk_4F5 & 4) && parent->subState == 1)
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        entity->scrollWithBg = 1;
        entity->state = 1;
        break;
    case 1:
        if (parent->unk_4F4.part8.unk_4F5 & 1)
        {
            entity->unk_500.part8.unk_500 = 1;
            entity->state = 2;
            entity->subState = 0;
            entity->timer = 0x64;
            entity->accelX.whole = parent->posX.whole - entity->posX.whole;
            entity->accelY.whole = parent->posY.whole - entity->posY.whole;
        }
        entity->posX.whole = target->posX.whole;
        entity->posY.whole = target->posY.whole;
        break;
    case 2:
        dx = (s16)(Target_GetPixelX(entity) - entity->posX.part16.integer);
        dy = (s16)(Target_GetPixelY(entity) - (u16)entity->posY.part16.integer);
        angle = ArcTan2(dx, dy);
        {
            s32 offsetX = Sin(angle + 0x4000);
            s32 x = entity->posX.whole;
            x += offsetX;
            entity->posX.whole = x;
        }
        {
            s32 offsetY = Sin(angle);
            s32 y = entity->posY.whole;
            y += offsetY;
            entity->posY.whole = y;
        }
        if (parent->state != 2) entity->state = entity->unk_500.part8.unk_500;
        break;
    }
}
