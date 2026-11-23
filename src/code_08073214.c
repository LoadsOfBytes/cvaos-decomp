#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void ManticoreTail_UpdateSwing(struct EwramData_EntityData *);
/**
 * @brief Tail tip update: anchor at the owner's back, swing through
 * ManticoreTail_UpdateSwing and update the affine matrix; delete when the
 * owner (type 0x36) is gone.
 */
void ManticoreTailTip_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    if (owner == NULL || owner->updateFunc == NULL || owner->unk_518.part8.enemyId != 0x36)
        goto remove;
    entity->posX.whole = owner->posX.whole;
    entity->posY.whole = owner->posY.whole;
    if (owner->flipX)
    {
        entity->velX.whole = -(((u8)entity->ext.manticoreTail.kind << 19) + 0xC0000);
        entity->posX.whole -= 0xC0000;
    }
    else
    {
        entity->velX.whole = ((u8)entity->ext.manticoreTail.kind << 19) + 0xC0000;
        entity->posX.whole += 0xC0000;
    }
    entity->posY.whole -= 0xE0000;
    entity->flipX = owner->flipX == 0;
    entity->hidden = owner->hidden;
    ManticoreTail_UpdateSwing(entity);
    entity->palette = owner->palette;
    ObjAffine_Set(entity->velY.whole, 0x10000, 0x10000, entity->flipX, entity->flipY, entity->affineIndex);
    return;
remove:
    ObjAffine_Free(entity->affineIndex);
    EntityDelete(entity);
}
