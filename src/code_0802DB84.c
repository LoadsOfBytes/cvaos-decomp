/* Soul effect helpers 0x0802DB84..0x0802DCFF. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void RedSoulAnchor_Update(struct EwramData_EntityData *entity);

/**
 * @brief Hit callback: unless the projectile already has an anchor (unk_504),
 * create a RedSoulAnchor_Update entity linking it to the hit target with
 * their current offset, and set hit flag 2.
 */
void RedSoul_AnchorToHitTarget(struct EwramData_EntityData *entity, struct EwramData_EntityData *target)
{
    if (entity->unk_504.unk_504_entity == NULL)
    {
        struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)RedSoulAnchor_Update);

        if (child != NULL)
        {
            entity->unk_504.unk_504_entity = child;
            child->unk_4F8.unk_4F8_entity = entity;
            child->unk_504.unk_504_entity = target;
            child->unk_4F4.part8.unk_4F4 = entity->unk_4F4.part8.unk_4F4;
            entity->hit.flags |= 2;
            child->accelX.whole = entity->posX.whole - target->posX.whole;
            child->accelY.whole = entity->posY.whole - target->posY.whole;
        }
    }
}

/**
 * @brief Draw callback of SkillCockatriceUse: draw 8 copies stepping by
 * (accelX, accelY) from 8 px below minus unk_508 * 8, then restore.
 */
void SkillCockatriceDraw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;
    s32 i;

    {
        s32 baseY = oldY + 0x80000;

        entity->posY.whole = baseY - entity->unk_508.whole * 8;
    }
    for (i = 8; i != 0; i--)
    {
        Sprite_Draw(entity);
        entity->posX.whole += entity->accelX.whole;
        entity->posY.whole += entity->accelY.whole;
    }
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}

/**
 * @brief Draw frames 0x10..0x14 at 5 points of the 10-entry room position
 * history (every second entry from unk_4F4).
 */
void RedSoulHistoryTrail_Draw(struct EwramData_EntityData *entity)
{
    s32 i = 0;
    u16 *history = (u16 *)&entity->unk_4F4;

    for (; i <= 4; i++)
    {
        struct BgLayerState *bg = &gEwramData->bgInfo[1];

        entity->posX.part16.integer = history[0] - bg->xPos.part16.integer;
        entity->posY.part16.integer = history[1] - bg->yPos.part16.integer;
        entity->animFrame = i + 0x10;
        Sprite_Draw(entity);
        history += 4;
    }
}

/**
 * @brief Draw frames 3..5 at 3 history points stored from unk_4F8.
 */
void SkillHarpyTrailDraw(struct EwramData_EntityData *entity)
{
    s32 i = 0;
    u16 *history = (u16 *)&entity->unk_4F4;

    for (; i <= 2; i++)
    {
        struct BgLayerState *bg = &gEwramData->bgInfo[1];

        entity->posX.part16.integer = history[2] - bg->xPos.part16.integer;
        entity->posY.part16.integer = history[3] - bg->yPos.part16.integer;
        entity->animFrame = i + 3;
        Sprite_Draw(entity);
        history += 2;
    }
}

/**
 * @brief Draw frames 2, 2, 3, 3 .. 5 at 8 history points stored from unk_4F8.
 */
void SkillSlimeTrailDraw(struct EwramData_EntityData *entity)
{
    s32 i = 0;
    u16 *history = (u16 *)&entity->unk_4F4;

    for (; i <= 7; i++)
    {
        struct BgLayerState *bg = &gEwramData->bgInfo[1];

        entity->posX.part16.integer = history[2] - bg->xPos.part16.integer;
        entity->posY.part16.integer = history[3] - bg->yPos.part16.integer;
        entity->animFrame = (i >> 1) + 2;
        Sprite_Draw(entity);
        history += 2;
    }
}
