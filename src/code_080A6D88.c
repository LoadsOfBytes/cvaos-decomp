/* Chaos2SpinningDebris_Update, Chaos2_SpawnShard, EnemyChaos2Update. */
#include "code_08009A0.h"
#include "chaos_entity.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"


extern s32 Entity_IsInRect(struct EwramData_EntityData *entity, s32 left, s32 top, s32 width, s32 height);
extern s32 Player_TrackEntity(struct EwramData_EntityData *entity);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern void Chaos2_UpdateIntro(struct EwramData_EntityData *entity);
extern void Chaos2_UpdateAttack(struct EwramData_EntityData *entity);
extern void Chaos2_UpdateIdle(struct EwramData_EntityData *entity);
extern void Chaos2_UpdateDefeat(struct EwramData_EntityData *entity);

/* Graphics of the parent (unk_504 points at it) used by its shards. */
struct ShardGfx {
    s32 gfx;
    s32 palette;
};

extern u8 sChaos2SpriteSheet[];
extern u32 RandomNumberGenerator(void);
extern void EntitySetVelocityFromAngle(struct EwramData_EntityData *entity, s32 angle, s32 speed);
void Chaos2Shard_Update(struct EwramData_EntityData *entity);

/**
 * @brief A6D88 | Update of a spinning projectile: angle unk_4FC += accelX,
 * move, rotate with its affine matrix affineIndex; once off screen release
 * the matrix (ObjAffine_Free) and delete.
 */
void Chaos2SpinningDebris_Update(struct EwramData_EntityData *entity)
{
    CHAOS_DEBRIS_ANGLE(entity) += entity->accelX.whole;
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    ObjAffine_Set((u16)CHAOS_DEBRIS_ANGLE(entity), 0x10000, 0x10000, entity->flipX, entity->flipY,
                 entity->affineIndex);
    if (EntityIsOutsideScreen(entity, 2))
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}

/**
 * @brief A6DEC | Spawn a shard of parent (slots 0x49..0xCF, update
 * Chaos2Shard_Update) at its position and layer with the parent's graphics
 * (unk_504), a random frame 0x4B..0x4D, random spin accelX and flips,
 * moving at angle/speed; unk_4F4 = parent.
 */
void Chaos2_SpawnShard(struct EwramData_EntityData *parent, s32 angle, s32 speed)
{
    struct EwramData_EntityData *shard = EntityCreateInRange(0x49, 0xCF, (u32 *)Chaos2Shard_Update);
    struct ShardGfx *gfx = (struct ShardGfx *)parent->unk_504.unk_504_entity;

    if (shard != NULL)
    {
        shard->posX.part16.integer = parent->posX.part16.integer;
        shard->posY.part16.integer = parent->posY.part16.integer;
        shard->spriteLayer = parent->spriteLayer;
        {
            s32 tiles = gfx->gfx;

            shard->unk_510.part8.basePalette = gfx->palette;
            Sprite_Attach(shard, sChaos2SpriteSheet, tiles, shard->unk_510.part8.basePalette);
        }
        shard->animFrame = RandomNumberGenerator() % 3 + 0x4B;
        shard->unk_518.part8.enemyId = parent->unk_518.part8.enemyId;
        CHAOS_LINK(shard) = parent;
        shard->scrollWithBg = 0;
        shard->accelX.whole = (RandomNumberGenerator() & 0x3FF) - 0x200;
        shard->flipX = RandomNumberGenerator() % 2;
        shard->flipY = RandomNumberGenerator() % 2;
        EntitySetVelocityFromAngle(shard, angle, speed);
        shard->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/**
 * @brief A6ED4 | Chaos (second form) update: outside state 3 react to the
 * player in range (Entity_IsInRect -> Player_TrackEntity) and Enemy_UpdateHitFlash; then
 * states 0..3 (Chaos2_UpdateIntro / Chaos2_UpdateAttack / Chaos2_UpdateIdle / Chaos2_UpdateDefeat),
 * animation and Enemy_CheckHitPlayer.
 */
void EnemyChaos2Update(struct EwramData_EntityData *entity)
{
    u8 unused[0x30]; /* stack frame reserved by the original */

    if (entity->state != 3)
    {
        if (Entity_IsInRect(entity, -0x10, 0, 0x110, 0xB0))
            Player_TrackEntity(entity);
        if (entity->state != 3)
            Enemy_UpdateHitFlash(entity);
    }
    switch (entity->state)
    {
        case 0:
            Chaos2_UpdateIntro(entity);
            break;
        case 1:
            Chaos2_UpdateAttack(entity);
            break;
        case 2:
            Chaos2_UpdateIdle(entity);
            break;
        case 3:
            Chaos2_UpdateDefeat(entity);
            break;
    }
    Anim_Update(entity);
    Enemy_CheckHitPlayer(entity);
}
