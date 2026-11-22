/* Creaking Skull part update and death motion. */
#include "code_08009A0.h"
#include "joint_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
#include "syscalls.h"

struct SkullBox { s8 x, y, w, h; };
extern void *memcpy(void *, const void *, unsigned long);
extern const u8 sCreakingSkullPartTimers[13];
extern void Hit_CheckWithPlayerSharedHistory(void *, struct EwramData_EntityData *, struct EwramData_EntityData *);
extern s32 Entity_GetVisibilityPattern(s32);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);
#define JOINT(e) ((s8)(e)->unk_504.part8.unk_504)
#define FIX_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)
#define HAS_SPRITE(e) ((e)->unk_504.part8.unk_504 <= 2 || JOINT(e) == 8 || JOINT(e) == 11)

/**
 * @brief Follow the selected root joint, mirror palette/facing and
 * maintain per-part hit boxes and affine orientation. On root death
 * enter the existing joint-dependent breakup motion.
 */
void CreakingSkullPart_Update(struct EwramData_EntityData *entity)
{
    u8 hit[16];
    u8 timers[13];
    struct SkullBox box;
    struct EwramData_EntityData *parent = entity->unk_500.unk_500_entity;
    struct EntityJointPart *parts;
    s32 x, y, size;
    u16 angle;

    memcpy(timers, sCreakingSkullPartTimers, sizeof(timers));
    if (entity->state == 0)
    {
        if (parent != NULL && parent->updateFunc != NULL
            && parent->unk_518.part8.enemyId == entity->unk_518.part8.enemyId)
        {
            parts = parent->unk_500.jointParts;
            entity->palette = parent->palette;
            entity->flipX = parent->flipX;
            entity->timer++;
            entity->posX.whole = parent->posX.whole + parts[JOINT(entity)].offsetX;
            entity->posY.whole = parent->posY.whole + parts[JOINT(entity)].offsetY;
            if (entity->unk_518.part8.enemyId == 0x5C && JOINT(entity) == 5)
                entity->hidden = 1;
            if (entity->unk_504.part8.unk_504 <= 1 || JOINT(entity) == 11)
            {
                x = parts[(s8)entity->unk_504.part8.unk_505].offsetX;
                x -= parts[JOINT(entity)].offsetX;
                y = parts[(s8)entity->unk_504.part8.unk_505].offsetY - parts[JOINT(entity)].offsetY;
                angle = ArcTan2(FIX_INT(x), FIX_INT(y));
                if (entity->flipX)
                    entity->velX.whole = angle;
                else
                    entity->velX.whole = angle - 0x8000;
                parts[JOINT(entity)].rotationAngle = entity->velX.whole;
            }
            else if (JOINT(entity) == 2 || JOINT(entity) == 8)
                entity->velX.whole = parts[JOINT(entity)].rotationAngle;
            if (parent->state == 2)
            {
                entity->state = 1;
                if (JOINT(entity) == 8)
                {
                    entity->state = 2;
                    entity->unk_4F8.whole = parent->posY.whole - 0xA0000;
                }
                entity->subState = 0;
                entity->unk_4FC.whole = entity->velX.whole;
                entity->unk_4F4.whole = (RandomNumberGenerator() & 0xFF) - 0x80;
                entity->accelY.whole = 0x1800;
                entity->accelX.whole = 0;
                entity->velX.whole = (RandomNumberGenerator() & 0x3FFF) - 0x2000;
                entity->velY.whole = 0;
                entity->timer = timers[JOINT(entity)];
                return;
            }
            if ((JOINT_HURT_MASK(parent) >> JOINT(entity)) & 1)
            {
                if (JOINT(entity) == 8)
                {
                    if (entity->flipX) box.x = -18;
                    else box.x = -20;
                    box.y = -22;
                    box.w = 30;
                    box.h = 32;
                    HitBox_SetAttack(entity, &box);
                    HitBox_SetBody(entity, &box);
                    entity->hit.flags &= 0xFB;
                    entity->hit.flags &= 0xFD;
                    Hit_CheckWithPlayerSharedHistory(hit, entity, parent);
                }
                else if (JOINT(entity) == 1)
                {
                    size = (entity->timer & 3) * 12;
                    if (entity->flipX)
                    {
                        x = Sin(-(parts[JOINT(entity)].rotationAngle + 0x8000) + 0x4000);
                        y = Sin(-(parts[JOINT(entity)].rotationAngle + 0x8000));
                    }
                    else
                    {
                        x = Sin(parts[JOINT(entity)].rotationAngle + 0xC000);
                        y = Sin(parts[JOINT(entity)].rotationAngle + 0x8000);
                    }
                    box.x = FIX_INT(x * size) - 3;
                    box.y = FIX_INT(y * size) - 3;
                    box.w = 6;
                    box.h = 6;
                    HitBox_SetAttack(entity, &box);
                    entity->hit.flags &= 0xFD;
                    entity->hit.flags &= 0xFB;
                    Hit_CheckWithPlayerSharedHistory(hit, entity, parent);
                }
                else if (JOINT(entity) == 2)
                {
                    size = ((entity->timer & 3) + 1) * 13;
                    if (entity->flipX)
                    {
                        x = Sin(-(parts[JOINT(entity)].rotationAngle + 0x8000) + 0x4000);
                        y = Sin(-(parts[JOINT(entity)].rotationAngle + 0x8000));
                    }
                    else
                    {
                        x = Sin(parts[JOINT(entity)].rotationAngle + 0xC000);
                        y = Sin(parts[JOINT(entity)].rotationAngle + 0x8000);
                    }
                    box.x = FIX_INT(x * size) - 3;
                    box.y = FIX_INT(y * size) - 3;
                    box.w = 6;
                    box.h = 6;
                    HitBox_SetAttack(entity, &box);
                    entity->hit.flags &= 0xFD;
                    entity->hit.flags |= 4;
                    Hit_CheckWithPlayerSharedHistory(hit, entity, parent);
                }
            }
            else
            {
                entity->hit.flags |= 4;
                entity->hit.flags |= 2;
            }
            if (HAS_SPRITE(entity))
                ObjAffine_Set(entity->velX.whole, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
            return;
        }
        if (HAS_SPRITE(entity)) ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
    else if (entity->state == 1)
    {
        if (entity->subState == 0)
        {
            entity->subState++;
            entity->scrollWithBg = 1;
        }
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if (entity->unk_518.part8.enemyId == 0x5C && JOINT(entity) == 5)
            entity->hidden = 1;
        if (entity->timer <= 50)
        {
            EntityAccelerateAndMove(entity, 0x60000);
            entity->unk_4FC.whole += entity->unk_4F4.whole;
        }
        if (parent->unk_514.part16.spawnParam0 == 0
            && (u8)(entity->timer - 20) <= 20 && (entity->timer & 3) == 0)
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x40000, 0x100000, 0x100000, entity->spriteLayer - 1);
        if (entity->unk_504.part8.unk_504 <= 2 || JOINT(entity) == 11)
            ObjAffine_Set(entity->unk_4FC.whole, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
        if ((s8)--entity->timer <= 0)
        {
            if (entity->unk_504.part8.unk_504 <= 2 || JOINT(entity) == 11)
                ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
        }
    }
    else if (entity->state == 2)
    {
        if (entity->subState == 0)
        {
            entity->subState++;
            entity->scrollWithBg = 1;
        }
        entity->hidden = 0;
        if (entity->subState == 1 && entity->timer <= 50)
        {
            EntityAccelerateAndMove(entity, 0x60000);
            if (entity->posY.whole >= entity->unk_4F8.whole)
            {
                entity->posY.whole = entity->unk_4F8.whole;
                entity->velY.whole = 0;
                entity->accelY.whole = 0;
                entity->velX.whole = 0;
                entity->accelX.whole = 0;
                entity->subState++;
            }
        }
        if (parent->unk_514.part16.spawnParam0 == 0
            && (u8)(entity->timer - 20) <= 20 && (entity->timer & 3) == 0)
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x40000, 0x100000, 0x100000, entity->spriteLayer - 1);
        if ((s8)--entity->timer <= 0) entity->timer = 1;
        if (entity->subState == 2)
        {
            entity->subState++;
            entity->stage = 0;
            entity->animFrame = 23;
            if (entity->flipX) entity->posX.whole += 0x110000;
            else entity->posX.whole -= 0x110000;
            entity->posY.whole += 0x80000;
        }
        else if (entity->subState == 3)
        {
            if (entity->stage == 0)
            {
                entity->stage++;
                PlaySong(0x7C);
            }
            if (entity->flipX)
            {
                if ((s32)entity->unk_4FC.whole > 0xE000) entity->unk_4FC.whole -= 0x400;
            }
            else if ((s32)entity->unk_4FC.whole < 0x12000) entity->unk_4FC.whole += 0x400;
        }
        ObjAffine_Set(entity->unk_4FC.whole, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    }
}
