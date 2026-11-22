/* Object 0x25 (Object25Update / Object25Create). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/* A spawn point of object 0x25 (16.16 world coordinates). */
struct Object25Point {
    s32 x;
    s32 y;
};

/* Per variant (unk_514): spawn point list and its length. */
extern const struct Object25Point *const sObject25Points[];
extern const u32 sObject25PointIndices[];
extern u8 sObject25AnimScript0[];
extern u8 sObject25AnimScript1[];

extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
void Object25_UpdateState(struct EwramData_EntityData *entity);
u8 Object25_LoadGraphics(struct EwramData_EntityData *entity);
void Object25Create(struct EwramData_EntityData *entity);

/**
 * @brief 579A4 | Object 0x25 update: state 0 -> Object25_UpdateState; state 1 is a
 * spawner that every 0x78 frames (50 %) creates up to 10 copies of itself
 * at a random point of its variant's list (screen relative to BG 1).
 */
void Object25Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct EwramData_EntityData *child;
    const struct Object25Point *points;
    u32 randomValue;
    u32 pointIndex;
    u32 var_6[4];

    switch (entity->state)
    {
        case 0:
            Object25_UpdateState(entity);
            break;
        case 1:
            if (entity->timer++ < 0x78)
                break;
            entity->timer = 0;
            randomValue = RandomNumberGenerator() % 2;
            if (randomValue != 0)
                break;
            if ((s32)entity->unk_4FC.whole > 9)
                break;
            child = EntityCreateInRange(0x1B, 0x48, (u32 *)EntityUpdateNothing);
            if (child == NULL)
                break;
            entity->unk_4FC.whole++;
            points = sObject25Points[entity->unk_514.part16.spawnParam0];
            pointIndex = sObject25PointIndices[entity->unk_514.part16.spawnParam0];
            pointIndex = RandomNumberGenerator() % pointIndex;
            child->posX.whole = points[pointIndex].x - bg->xPos.whole;
            child->posY.whole = points[pointIndex].y - bg->yPos.whole;
            child->scrollWithBg = 1;
            child->unk_518.part8.spawnIndex = randomValue;
            child->unk_518.part8.enemyId = entity->unk_518.part8.enemyId;
            child->unk_514.part16.spawnParam1 = randomValue;
            child->unk_514.part16.spawnParam0 = entity->unk_514.part16.spawnParam0;
            child->drawFunc = (u32 *)Sprite_DrawEntity;
            child->updateFunc = entity->updateFunc;
            Object25Create(child);
            child->unk_500.unk_500_entity = entity;
            break;
    }
}

/**
 * @brief 57AA0 | Object 0x25 create. Variant unk_516 == 1 becomes the
 * spawner (state 1); otherwise load graphics and set up; unk_516 == 2
 * starts moving at once. Deleted when bossRushMode == 0 and bossEventFlags bit 5.
 */
void Object25Create(struct EwramData_EntityData *entity)
{
    if (gEwramData->gameState.bossRushMode == 0 && (gEwramData->gameState.bossEventFlags & 0x20))
    {
        EntityDelete(entity);
        return;
    }
    if (entity->unk_514.part16.spawnParam1 == 1)
    {
        entity->state = 1;
        entity->hidden = 1;
        entity->timer = 0x78;
        entity->hit.flags |= 4;
        entity->hit.flags |= 2;
        Entity_SnapToGround(entity);
        return;
    }
    if (!Object25_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->flipX = 0;
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    Anim_Start(entity, (struct AnimScript *)sObject25AnimScript0, 0, 0);
    Anim_Update(entity);
    entity->enemyFlags |= 1;
    entity->attack = 0x28;
    entity->unk_518.part16.hp = 0xC8;
    entity->spriteLayer = 0x14;
    Entity_SnapToGround(entity);
    entity->state = 0;
    if (entity->unk_514.part16.spawnParam1 == 2)
    {
        entity->subState = 1;
        Anim_Start(entity, (struct AnimScript *)sObject25AnimScript1, 0, 1);
        Anim_Update(entity);
        entity->velX.whole = 0x4000;
        entity->velY.whole = 0x40000;
        entity->spriteLayer--;
    }
}
