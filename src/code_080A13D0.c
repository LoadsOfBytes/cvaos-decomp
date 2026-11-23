#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern s32 Entity_GetVisibilityPattern(s32 step);
extern void EntityDampVelocityX(struct EwramData_EntityData *entity, s32 factor);

/**
 * @brief A13D0 | Gladiator spinning part: follows the owner like
 * GladiatorBody_Update, spins its affine matrix by unk_502 (accelerating by 0x18 up
 * to 0x800), visible only during the owner's animation 0xC. When the owner
 * dies it falls (velY -0xC000, gravity 0x1000) still spinning, blinks for
 * 0x40 frames and releases its affine slot.
 */
void GladiatorSpinner_Update(struct EwramData_EntityData *entity)
{
    u8 unused[0x10];
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);

    if (entity->state == 0)
    {
        if (owner == NULL || owner->updateFunc == NULL || owner->enemyId != 0x5D)
            goto delete;
        if (owner->palette == 4)
            entity->palette = 4;
        else
            entity->palette = entity->basePalette;
        entity->flipX = owner->flipX;
        entity->posX.part16.integer = owner->posX.part16.integer;
        entity->posY.part16.integer = owner->posY.part16.integer;
        if (entity->flipX)
            entity->posX.part16.integer = entity->posX.part16.integer - entity->ext.gladiatorSpinner.offsetX;
        else
            entity->posX.part16.integer += entity->ext.gladiatorSpinner.offsetX;
        entity->posY.part16.integer += entity->ext.gladiatorSpinner.offsetY;
        Anim_Update(entity);
        if (entity->flipX)
            entity->ext.gladiatorSpinner.angle += entity->ext.gladiatorSpinner.spin;
        else
            entity->ext.gladiatorSpinner.angle = entity->ext.gladiatorSpinner.angle - entity->ext.gladiatorSpinner.spin;
        if ((u16)entity->ext.gladiatorSpinner.spin < 0x800)
            entity->ext.gladiatorSpinner.spin += 0x18;
        ObjAffine_Set((u16)entity->ext.gladiatorSpinner.angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
        if (owner->animScriptIndex == 0xC)
        {
            entity->hidden = 0;
        }
        else
        {
            entity->hidden = 1;
            entity->ext.gladiatorSpinner.spin = 0;
        }
        if (owner->state == 1)
        {
            entity->state = 1;
            entity->subState = 0;
            entity->timer = 0x40;
            entity->velX.whole = owner->velX.whole;
            entity->velY.whole = -0xC000;
            entity->accelX.whole = 0;
            entity->accelY.whole = 0x1000;
            if (entity->hidden)
            {
                ObjAffine_Free(entity->affineIndex);
            delete:
                EntityDelete(entity);
            }
        }
        return;
    }
    EntityDampVelocityX(entity, 0x20);
    EntityAccelerateAndMove(entity, 0x50000);
    if (entity->flipX)
        entity->ext.gladiatorSpinner.angle += entity->ext.gladiatorSpinner.spin;
    else
        entity->ext.gladiatorSpinner.angle = entity->ext.gladiatorSpinner.angle - entity->ext.gladiatorSpinner.spin;
    ObjAffine_Set((u16)entity->ext.gladiatorSpinner.angle, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    entity->hidden = Entity_GetVisibilityPattern(entity->timer);
    if ((s8)--entity->timer <= 0)
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
    }
}
