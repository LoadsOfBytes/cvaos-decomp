/* Effect_DrawAtOffsetPair. */
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

/**
 * @brief Draw once at the entity position and once at the stored secondary offset, then restore the position.
 */
void Effect_DrawAtOffsetPair(struct EwramData_EntityData *entity)
{
    Sprite_Draw(entity);
    entity->posX.whole += entity->ext.particlePair.offsetX;
    entity->posY.whole += entity->ext.particlePair.offsetY;
    Sprite_Draw(entity);
    entity->posX.whole -= entity->ext.particlePair.offsetX;
    entity->posY.whole -= entity->ext.particlePair.offsetY;
}

/**
 * @brief Stay hidden through the initial countdown, then accelerate and move on both axes and request deletion when the animation returns three.
 */
void Effect_UpdateDelayedParticle(struct EwramData_EntityData *entity)
{
    if (entity->state == 0)
    {
        entity->hidden = 1;
        if (entity->timer != 0)
        {
            entity->timer--;
            return;
        }
        entity->hidden = 0;
        entity->state++;
    }
    EntityAccelerateAndMoveX(entity, 0x80000);
    EntityAccelerateAndMoveY(entity, 0x80000);
    if (Anim_Update(entity) == 3)
        entity->deleteRequested = 1;
}

extern s32 Pickup_TrySpawnOnHit(struct EwramData_EntityData *entity, s32 offsetY, u8 kind, s16 amount);
extern void Effect_SpawnRandom(s32 x, s32 y, s32 width, s32 height, s32 layer);

/**
 * @brief Hide off screen, spawn a reward on a qualifying player hit, then hide, emit the existing particles and expire the timer.
 */
void PickupSource_Update(struct EwramData_EntityData *entity)
{
    if (Entity_IsInRect(entity, -0x10, -0x10, 0x110, 0xC0) == 0)
    {
        entity->hidden = 1;
        return;
    }
    switch (entity->state)
    {
        case 0:
            entity->hidden = 0;
            if (Pickup_TrySpawnOnHit(entity, 0, entity->unk_518.part8.enemyId, entity->unk_514.part16.spawnParam1) != 0)
            {
                PlaySong(0xA2);
                entity->hidden = 1;
                entity->timer = 0x28;
                entity->state++;
            }
            break;
        case 1:
            if (entity->timer > 0x1E && (entity->timer & 3) == 0)
                Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0x80000, 0x80000, entity->spriteLayer - 1);
            if (--entity->timer == 0xFF)
                entity->deleteRequested = 1;
            break;
    }
}
