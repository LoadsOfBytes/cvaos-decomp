/* Great Armor part update: joint following, weapon collision and death motion. */
#include "code_08009A0.h"
#include "great_armor_entity.h"
#include "joint_entity.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "syscalls.h"

struct ArmorBox { s8 x, y, w, h; };
extern void *memcpy(void *, const void *, unsigned long);
extern const u8 sGreatArmorPartTimers[14];
extern const s16 sGreatArmorPartOffsets[14];
extern void Hit_CheckWithPlayerSharedHistory(void *, struct EwramData_EntityData *, struct EwramData_EntityData *);
extern u32 Hit_CheckWithPlayer(void *, void *);
extern s32 Entity_GetVisibilityPattern(s32);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);
#define JOINT(e) ((s8)JOINT_PART_INDEX((e)))
#define FIX_INT(v) ((v) < 0 ? -(-(v) >> 16) : (v) >> 16)
#define HAS_SPRITE(e) (JOINT(e) == 2 || JOINT(e) == 5 || JOINT(e) == 8 || JOINT(e) == 11 || JOINT(e) == 12 || JOINT(e) == 13)

/**
 * @brief Part entity update: follow its joint of the parent skeleton, weapon
 * collision and the death motion.
 */
void GreatArmorPart_Update(struct EwramData_EntityData *entity)
{
    u8 hit[16];
    u8 timers[14];
    s16 offsets[14];
    struct ArmorBox box;
    struct EwramData_EntityData *parent = JOINT_PART_OWNER(entity);
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct GreatArmorPart *parts;
    s32 x, y, size;

    memcpy(timers, sGreatArmorPartTimers, sizeof(timers));
    memcpy(offsets, sGreatArmorPartOffsets, sizeof(offsets));
    if (entity->state == 0)
    {
        if (parent != NULL && parent->updateFunc != NULL
            && parent->unk_518.part8.enemyId == entity->unk_518.part8.enemyId)
        {
            parts = parent->unk_500.greatArmorParts;
            entity->palette = parent->palette;
            entity->flipX = parent->flipX;
            entity->timer++;
            entity->posX.whole = parent->posX.whole + parts[JOINT(entity)].offsetX;
            entity->posY.whole = parent->posY.whole + parts[JOINT(entity)].offsetY;
            if (JOINT(entity) == 2 || JOINT(entity) == 8 || JOINT(entity) == 11)
            {
                x = parts[(s8)JOINT_PART_ANCHOR(entity)].offsetX;
                x -= parts[JOINT(entity)].offsetX;
                /* Keep both coordinate loads separate for matching agbcc allocation. */
                y = parts[(s8)JOINT_PART_ANCHOR(entity)].offsetY;
                y -= parts[JOINT(entity)].offsetY;
                entity->velX.whole = ArcTan2(FIX_INT(x), FIX_INT(y)) - 0x4000;
            }
            else if (JOINT(entity) == 5 || JOINT(entity) == 12 || JOINT(entity) == 13)
                entity->velX.whole = parts[JOINT(entity)].rotationAngle;
            if (parent->state == 2)
            {
                entity->state = 1;
                entity->subState = 0;
                GREAT_ARMOR_PART_ANGLE(entity) = entity->velX.whole;
                GREAT_ARMOR_PART_SPIN(entity) = (RandomNumberGenerator() & 0xFF) - 0x80;
                GREAT_ARMOR_PART_FLOOR_Y(entity) = parent->posY.part16.integer + (s16)camera->yPos.part16.integer + offsets[JOINT(entity)];
                entity->accelY.whole = 0x1800;
                entity->accelX.whole = 0;
                entity->velX.whole = (RandomNumberGenerator() & 0x3FFF) - 0x2000;
                entity->velY.whole = 0;
                entity->timer = timers[JOINT(entity)];
                return;
            }
            if ((JOINT_HURT_MASK(parent) >> JOINT(entity)) & 1)
            {
                if (JOINT(entity) == 0)
                    Hit_CheckWithPlayer(hit, entity);
                else if (JOINT(entity) == 12)
                {
                    size = ((entity->timer & 3) + 1) * 14;
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
                    if ((entity->timer & 3) == 0)
                    {
                        box.x = FIX_INT(size * x) - 6;
                        box.y = FIX_INT(size * y) - 6;
                        box.w = 12;
                        box.h = 16;
                    }
                    else
                    {
                        box.x = FIX_INT(size * x) - 4;
                        box.y = FIX_INT(size * y) - 4;
                        box.w = 8;
                        box.h = 8;
                    }
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
            if (entity->posY.part16.integer < -24) entity->hidden = 1;
            else if (entity->posY.part16.integer > 192) entity->hidden = 1;
            else entity->hidden = 0;
            return;
        }
        if (HAS_SPRITE(entity)) ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
    else
    {
        if (entity->subState == 0)
        {
            entity->subState++;
            entity->scrollWithBg = 1;
            if (JOINT(entity) == 12) entity->subState = 10;
        }
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if (entity->timer <= 60)
        {
            EntityAccelerateAndMove(entity, 0x60000);
            GREAT_ARMOR_PART_ANGLE(entity) += GREAT_ARMOR_PART_SPIN(entity);
            if (entity->subState == 1
                && entity->posY.part16.integer + (s16)camera->yPos.part16.integer >= GREAT_ARMOR_PART_FLOOR_Y(entity))
            {
                entity->velX.whole = 0;
                entity->accelX.whole = 0;
                entity->velY.whole = 0;
                entity->accelY.whole = 0;
                GREAT_ARMOR_PART_SPIN(entity) = 0;
                entity->subState++;
                if ((u8)(JOINT_PART_INDEX(entity) - 1) > 2
                    && JOINT_PART_INDEX(entity) != 7)
                    PlaySong(0x7F);
            }
        }
        if (entity->subState == 2 && parent->unk_514.part16.spawnParam0 == 0 && (entity->timer & 7) == 0)
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x40000, 0x100000, 0x100000, entity->spriteLayer - 1);
        if (HAS_SPRITE(entity))
            ObjAffine_Set(GREAT_ARMOR_PART_ANGLE(entity), 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
        if (--entity->timer <= 1)
        {
            if (HAS_SPRITE(entity)) ObjAffine_Free(entity->affineIndex);
            if (JOINT(entity) == 4) PlaySong(0x6E);
            EntityDelete(entity);
        }
        else if (!entity->hidden)
        {
            if (entity->posY.part16.integer < -24) entity->hidden = 1;
            else if (entity->posY.part16.integer > 240) entity->hidden = 1;
        }
    }
}
