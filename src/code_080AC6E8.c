/* Big Golem part update: joint following, weapon collision and death motion. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "syscalls.h"

struct GolemBox { s8 x, y, w, h; };
extern void *memcpy(void *, const void *, unsigned long);
extern const u8 sBigGolemPartTimers[14];
extern const s16 sBigGolemPartOffsets[14];
extern void Hit_CheckWithPlayerSharedHistory(void *, struct EwramData_EntityData *, struct EwramData_EntityData *);
extern s32 Entity_GetVisibilityPattern(s32);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);
#define JOINT(e) ((s8)e->ext.jointPart.index)
#define FIX_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)
#define HAS_SPRITE(e) (JOINT(e) == 2 || JOINT(e) == 4 || JOINT(e) == 5 || JOINT(e) == 8 || JOINT(e) == 12 || JOINT(e) == 13)
#define HAD_SPRITE(e) (JOINT(e) == 2 || JOINT(e) == 5 || JOINT(e) == 8 || JOINT(e) == 11 || JOINT(e) == 12 || JOINT(e) == 13)

/**
 * @brief Part entity update: follow its joint of the parent golem (palette,
 * flip, position), handle the weapon/contact boxes and the death motion.
 */
void BigGolemPart_Update(struct EwramData_EntityData *entity)
{
    u8 hit[16];
    u8 timers[14];
    s16 offsets[14];
    struct GolemBox box;
    struct EwramData_EntityData *parent = entity->ext.bigGolemPart.owner;
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct BigGolemPart *parts;
    s32 x, y, size;

    memcpy(timers, sBigGolemPartTimers, sizeof(timers));
    memcpy(offsets, sBigGolemPartOffsets, sizeof(offsets));
    if (entity->state == 0)
    {
        if (parent != NULL && parent->updateFunc != NULL
            && parent->unk_518.part8.enemyId == entity->unk_518.part8.enemyId)
        {
            parts = parent->ext.bigGolem.parts;
            if (parent->palette == 4)
                entity->palette = 4;
            else
                entity->palette = entity->unk_510.part8.basePalette;
            entity->flipX = parent->flipX;
            entity->timer++;
            entity->posX.whole = parent->posX.whole + parts[JOINT(entity)].offsetX;
            entity->posY.whole = parent->posY.whole + parts[JOINT(entity)].offsetY;
            if (JOINT(entity) == 2 || JOINT(entity) == 4 || JOINT(entity) == 8 || JOINT(entity) == 13)
            {
                u16 angle;

                x = parts[(s8)entity->ext.jointPart.anchor].offsetX;
                x -= parts[JOINT(entity)].offsetX;
                y = parts[(s8)entity->ext.jointPart.anchor].offsetY;
                y -= parts[JOINT(entity)].offsetY;
                angle = ArcTan2(FIX_INT(x), FIX_INT(y));
                if (JOINT(entity) == 4)
                {
                    if (entity->flipX)
                        entity->velX.whole = angle + 0x2700;
                    else
                        entity->velX.whole = angle + 0x5900;
                }
                else
                    entity->velX.whole = angle - 0x4000;
                parts[JOINT(entity)].rotationAngle = entity->velX.whole;
            }
            else if (JOINT(entity) == 5 || JOINT(entity) == 12)
                entity->velX.whole = parts[JOINT(entity)].rotationAngle;
            if (parent->state == 2)
            {
                entity->state = 1;
                entity->subState = 0;
                entity->ext.bigGolemPart.angle = entity->velX.whole;
                entity->ext.bigGolemPart.spin = (RandomNumberGenerator() & 0xFF) - 0x80;
                entity->ext.bigGolemPart.floorY = parent->posY.part16.integer + (s16)camera->yPos.part16.integer + offsets[JOINT(entity)];
                entity->accelY.whole = 0x1800;
                entity->accelX.whole = 0;
                entity->velX.whole = (RandomNumberGenerator() & 0x3FFF) - 0x2000;
                entity->velY.whole = 0;
                entity->timer = timers[JOINT(entity)];
                return;
            }
            if ((parent->ext.joint.hurtMask >> JOINT(entity)) & 1)
            {
                if (JOINT(entity) == 2 || JOINT(entity) == 8 || JOINT(entity) == 13)
                {
                    size = ((entity->timer & 1) + 1) * 21;
                    if (entity->flipX)
                    {
                        x = Sin(0x8000 - parts[JOINT(entity)].rotationAngle);
                        y = Sin(0x4000 - parts[JOINT(entity)].rotationAngle);
                    }
                    else
                    {
                        x = Sin(parts[JOINT(entity)].rotationAngle + 0x8000);
                        y = Sin(parts[JOINT(entity)].rotationAngle + 0x4000);
                    }
                    box.x = FIX_INT(size * x) - 10;
                    box.y = FIX_INT(size * y) - 10;
                    box.w = 20;
                    box.h = 20;
                    HitBox_SetAttack(entity, &box);
                    HitBox_SetBody(entity, &box);
                    HitBox_SetBody(parent, &box);
                    entity->hit.flags &= 0xFB;
                    entity->hit.flags &= 0xFD;
                    Hit_CheckWithPlayerSharedHistory(hit, entity, parent);
                }
                else if (JOINT(entity) == 3)
                {
                    box.x = -16;
                    box.y = -64;
                    box.w = 32;
                    box.h = 90;
                    HitBox_SetAttack(entity, &box);
                    HitBox_SetBody(entity, &box);
                    HitBox_SetBody(parent, &box);
                    entity->hit.flags &= 0xFB;
                    entity->hit.flags &= 0xFD;
                    Hit_CheckWithPlayerSharedHistory(hit, entity, parent);
                }
                else if (JOINT(entity) == 5)
                {
                    box.x = -12;
                    box.y = -10;
                    box.w = 24;
                    box.h = 48;
                    HitBox_SetAttack(entity, &box);
                    HitBox_SetBody(entity, &box);
                    HitBox_SetBody(parent, &box);
                    entity->hit.flags &= 0xFB;
                    entity->hit.flags &= 0xFD;
                    Hit_CheckWithPlayerSharedHistory(hit, entity, parent);
                }
                else if (JOINT(entity) == 12)
                {
                    size = 21;
                    if (entity->flipX)
                    {
                        x = Sin(0x8000 - parts[JOINT(entity)].rotationAngle);
                        y = Sin(0x4000 - parts[JOINT(entity)].rotationAngle);
                    }
                    else
                    {
                        x = Sin(parts[JOINT(entity)].rotationAngle + 0x8000);
                        y = Sin(parts[JOINT(entity)].rotationAngle + 0x4000);
                    }
                    box.x = FIX_INT(size * x) - 9;
                    box.y = FIX_INT(size * y) - 9;
                    box.w = 18;
                    box.h = 18;
                    HitBox_SetAttack(entity, &box);
                    HitBox_SetBody(entity, &box);
                    HitBox_SetBody(parent, &box);
                    entity->hit.flags &= 0xFD;
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
        if (HAD_SPRITE(entity)) ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
    else
    {
        if (entity->subState == 0)
        {
            entity->subState++;
            entity->scrollWithBg = 1;
            if (JOINT(entity) == 13) entity->subState = 10;
        }
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if (entity->timer <= 60)
        {
            EntityAccelerateAndMove(entity, 0x60000);
            entity->ext.bigGolemPart.angle += entity->ext.bigGolemPart.spin;
            if (entity->subState == 1
                && entity->posY.part16.integer + (s16)camera->yPos.part16.integer >= entity->ext.bigGolemPart.floorY)
            {
                entity->velX.whole = 0;
                entity->accelX.whole = 0;
                entity->velY.whole = 0;
                entity->accelY.whole = 0;
                entity->ext.bigGolemPart.spin = 0;
                entity->subState++;
                if ((u8)(entity->ext.jointPart.index - 1) > 2
                    && entity->ext.jointPart.index != 7)
                    PlaySong(0x71);
            }
        }
        if (entity->subState == 2 && parent->unk_514.part16.spawnParam0 == 0 && (entity->timer & 7) == 0)
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x40000, 0x100000, 0x100000, entity->spriteLayer - 1);
        if (HAS_SPRITE(entity))
            ObjAffine_Set(entity->ext.bigGolemPart.angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
        if (--entity->timer <= 1)
        {
            if (HAS_SPRITE(entity)) ObjAffine_Free(entity->affineIndex);
            if (JOINT(entity) == 5) PlaySong(0x6E);
            EntityDelete(entity);
        }
        else if (!entity->hidden)
        {
            if (entity->posY.part16.integer < -24) entity->hidden = 1;
            else if (entity->posY.part16.integer > 240) entity->hidden = 1;
        }
    }
}
