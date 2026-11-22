#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

#define OWNER_DONE(o) ((o)->unk_4FC.part8.unk_4FE & 1)

/* Effect circling its anchor (unk_4FC/unk_500, camera-relative) with a growing
 * radius; deleted when flagged or when its source (unk_4F4) is done. */
/**
 * @brief Orbit the stored anchor with increasing angular speed, expand
 * the radius after timer 25 and compensate for camera movement.
 * Advance animation every other update; remove on completion/cancel.
 */
void HeadhunterOrbitingParticle_Update(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    struct EwramData_EntityData *source = entity->unk_4F4.unk_4F4_entity;
    s32 x, y, radius;
    if (entity->accelY.whole <= 0x3FF)
        entity->accelY.whole += 0x10;
    entity->accelX.whole += entity->accelY.whole;
    radius = 0x4000;
    x = Sin(entity->accelX.whole + radius);
    y = Sin(entity->accelX.whole);
    if (entity->timer <= 25)
    {
        radius = 0x200000;
        x = FixedMultiply(x, radius);
        y = FixedMultiply(y, radius);
    }
    else
    {
        s32 offset = (entity->timer - 26) << 16;
        s32 base = 0x200000;
        x = FixedMultiply(x, offset + base);
        y = FixedMultiply(y, ((entity->timer - 26) << 16) + base);
    }
    entity->posX.whole = entity->velX.whole + x;
    entity->posY.whole = entity->velY.whole + y;
    entity->posX.whole += entity->unk_4FC.whole - camera->xPos.whole;
    entity->posY.whole += entity->unk_500.whole - camera->yPos.whole;
    if (entity->timer++ & 1)
        Anim_Update(entity);
    if (entity->animDone)
        EntityDelete(entity);
    else if (OWNER_DONE(source))
        EntityDelete(entity);
}
