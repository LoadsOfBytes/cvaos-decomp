/* Man Eater update (death burst, chase, crumbling pieces).: the piece's flipX is written through an s8 byte
 * pointer with a u8 masked copy (keeps the mask before the address and the
 * global-allocation priority of entity/child as in the original). */
#include "code_08009A0.h"
#include "code_08013960.h"
#include "code_08033CAC.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern u8 sManEaterGfx[], sManEaterPalette[], sManEaterSpriteSheet[];
extern const u8 sUnk_085288A8[][4];

#include "code_080D73B8.h"
#include "code/code_080211F0.h"
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern void BossDefeatEffect_Spawn(s32, s32);
extern void BossRecoveryPickup_Spawn(s32, s32, s32, s32, s32);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 Entity_StepTowardPoint(struct EwramData_EntityData *, s32, s32, s32);
extern void ManEaterHead_Update(struct EwramData_EntityData *);
extern void ManEaterDebris_Update(struct EwramData_EntityData *);

static inline void SetFlipX(struct EwramData_EntityData *entity, u32 flipped)
{
    entity->flipX = flipped;
}

void EnemyManEaterUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    s32 angle;
    s32 x;
    s32 y;
    s32 offset;
    if (entity->state != 3 && (s8)entity->ext.manEaterBody.headCount == 0)
    {
        entity->animFrame = 0x11;
        Enemy_DropRewards(entity);
        entity->state = 3;
        entity->timer = 0x80;
        entity->accelY.whole = 0x400;
        PlaySong(0x6B);
        if (entity->spawnParam0 != 0)
        {
            Player_SetSelectedTarget(NULL);
            BossDefeatEffect_Spawn(entity->posX.whole, entity->posY.whole - 0x200000);
            BossRecoveryPickup_Spawn((s16)entity->posX.part16.integer, 0x20, 0xD, 0x60, 0);
        }
    }
    switch (entity->state)
    {
    case 0:
        if (!Target_IsNearEntity(entity, 0xA0, 0x78)) return;
        entity->state = 1;
        entity->timer = 0x30;
        entity->subState = 0;
        break;
    case 1:
        if (entity->timer != 0) entity->timer--;
        else
        {
            child = Entity_CreateObject(ManEaterHead_Update);
            if (child == NULL) return;
            child->ext.manEater.ownerIndex = ENTITY_INDEX(entity);
            {
                s32 value = ((entity->subState + 1) << 16) / 3;
                value -= 0x4000;
                child->ext.manEater.angle = value;
                child->ext.manEater.baseAngle = value;
                child->ext.manEater.homeAngle = value;
            }
            child->ext.manEater.orbitSlot = entity->subState;
            {
                s32 offsetX = Sin(child->ext.manEater.angle + 0x4000) << 4;
                s32 x = entity->posX.whole;
                x += offsetX;
                child->posX.whole = x;
            }
            {
                s32 offsetY = Sin(child->ext.manEater.angle) << 4;
                s32 y = entity->posY.whole;
                y += offsetY;
                child->posY.whole = y;
            }
            entity->timer = 0x30;
            if (++entity->subState > 2) entity->state = 2;
        }
        break;
    case 2:
        offset = 0x200000;
        if (entity->posX.whole < Target_GetX(entity)) offset = -offset;
        x = Target_GetX(entity) + offset;
        y = Target_GetY(entity) - 0x300000;
        Entity_StepTowardPoint(entity, x, y, 8);
        break;
    case 3:
        if (!(entity->timer & 3))
        {
            child = EntityCreateInRange(0x49, 0xCF, (u32 *)ManEaterDebris_Update);
            if (child != NULL)
            {
                {
                    s32 offsetX = RandomSigned(0x200000);
                    s32 x = entity->posX.whole;
                    x += offsetX;
                    child->posX.whole = x;
                }
                {
                    s32 offsetY = RandomSigned(0x200000);
                    s32 y = entity->posY.whole;
                    y += offsetY;
                    child->posY.whole = y;
                }
                child->velX.whole = RandomSigned(0x20000);
                child->velY.whole = RandomSigned(0x8000) - 0x18000;
                child->accelY.whole = 0x2000;
                child->spriteLayer = entity->spriteLayer;
                Entity_LoadGraphics(child, sManEaterGfx, sManEaterPalette, 0, sManEaterSpriteSheet);
                child->animFrame = RandomNumberGenerator() % 3 + 0x12;
                { s32 f = RandomNumberGenerator() & 1; s8 *p = (s8 *)child + 0x58; s32 t = f << 6; u8 m = *p & ~0x40; *p = m | t; }
                child->flipY = RandomNumberGenerator() % 2;
                child->scrollWithBg = 1;
                child->timer = (RandomNumberGenerator() & 0xF) + 0x28;
                child->subState = 1;
            }
        }
        EntityAccelerateAndMove(entity, 0x80000);
        if (entity->timer == 0)
        {
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
        }
        if (--entity->timer == 0x30) entity->drawSkip = 1;
        break;
    }
    entity->ext.manEaterBody.swayPhase += 0x80;
    entity->ext.manEaterBody.angle = (Sin(entity->ext.manEaterBody.swayPhase) / 256) << 4;
    ObjAffine_Set(entity->ext.manEaterBody.angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
}
