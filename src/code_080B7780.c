/* Rotating attached part: follow the parent during animation 3, then fall
 * and release its affine slot when the parent dies. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/entity_template.h"

/**
 * @brief Follow the owner and rotate while its animation 3 is active.
 * On owner death, detach and fall; release the affine slot when removed.
 * The visual identity of this attached part is unresolved.
 */
void SkullMillioneFamilyPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    u16 offset;
    s32 rotation;
    s32 value;
    if (entity->state <= 1)
    {
        if (parent->state == 3) entity->state = 2;
        else if (gEwramData->gameState.timeStopTimer != 0)
        {
            struct EntityTemplate *entry = (struct EntityTemplate *)(parent->unk_518.part8.enemyId * sizeof(*entry) + (u32)sEnemyData);
            if (!(entry->flags_1C & 0x400)) return;
        }
    }
    switch (entity->state)
    {
    case 0:
        if (parent->animScriptIndex != 3) break;
        entity->ext.skullMillionePart.angle = 0;
        entity->hidden = 0;
        entity->state = 1;
        /* fall through */
    case 1:
        if (parent->animScriptIndex != 3)
        {
            entity->hidden = 1;
            entity->state = 0;
            break;
        }
        entity->flipX = parent->flipX;
        offset = -12;
        rotation = 0xEE00;
        if (entity->flipX)
        {
            offset = 12;
            rotation = -(s16)rotation;
        }
        {
            u16 *position = &parent->posX.part16.integer;
            value = (s16)offset;
            value += *position;
            entity->posX.part16.integer = value;
        }
        entity->posY.part16.integer = parent->posY.part16.integer - 6;
        value = (s16)rotation;
        value += (u16)entity->ext.skullMillionePart.angle;
        entity->ext.skullMillionePart.angle = value;
        break;
    case 2:
        entity->hidden = 0;
        entity->flipX = parent->flipX;
        offset = -12;
        if (entity->flipX) offset = 12;
        {
            u16 *position = &parent->posX.part16.integer;
            value = (s16)offset;
            value += *position;
            entity->posX.part16.integer = value;
        }
        entity->posY.part16.integer = parent->posY.part16.integer - 6;
        entity->velY.whole = (RandomSigned(0x10) << 12) - 0x38000;
        entity->accelY.whole = 0x2000;
        entity->velX.whole = RandomSigned(0x40) << 12;
        entity->accelX.whole = 0;
        entity->ext.skullMillionePart.angle = 0;
        entity->state = 3;
        /* fall through */
    case 3:
        rotation = 0xB00;
        if (entity->flipX) rotation = (u16)-rotation;
        value = (s16)rotation;
        value += (u16)entity->ext.skullMillionePart.angle;
        entity->ext.skullMillionePart.angle = value;
        EntityAccelerateAndMove(entity, 0x80000);
        if (!EntityIsOutsideScreen(entity, 4)) break;
        /* fall through */
    case 4:
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        return;
    }
    ObjAffine_Set(entity->ext.skullMillionePart.angle, 0x10000, 0x10000, entity->flipX, 0, entity->affineIndex);
}
