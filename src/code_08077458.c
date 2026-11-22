/* Linked body segments follow the owner's palette and stagger their destruction.
 * The r6 visibility pointer preserves agbcc's field-pointer allocation.
 * This constraint does not establish original source spelling. */
#include "code_08009A0.h"
#include "white_dragon_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
extern void WhiteDragon_SpawnSpark(struct EwramData_EntityData *,s32,s32,s32);
extern void WhiteDragon_SpawnDeathPiece(struct EwramData_EntityData *,s32,s32,s32);
extern s32 Enemy_IsStunnedNoFlash(struct EwramData_EntityData *);
#define STATE_WORD(e) (*(u16 *)&(e)->state)
#define SEGMENT(e) (*(s8 *)&WHITE_DRAGON_SEGMENT_INDEX((e)))
struct SegmentFlags { u8 slot:5; u8 hidden:1; u8 flipX:1; u8 flipY:1; };
/**
 * @brief Neck segment update: copy the owner's (type 11) visibility and palette,
 * throw sparks while it dies; delete when the owner is gone.
 */
void WhiteDragonSegment_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(entity);
    if (owner && owner->updateFunc && owner->unk_518.part8.enemyId==11)
    {
        struct SegmentFlags *visibility;
        u8 hidden = owner->hidden;
        visibility = (struct SegmentFlags *)(ENTITY_RENDER_FLAGS_ADDRESS(entity));
        visibility->hidden = hidden;
        do { entity->palette = owner->palette; } while (0);
        if (STATE_WORD(owner)==0x102)
        {
            visibility->hidden = 0;
            if (SEGMENT(entity)<=5 && !(owner->timer & 7))
                WhiteDragon_SpawnSpark(entity,entity->posX.whole,entity->posY.whole,entity->spriteLayer-1);
            if ((owner->timer>>2)==SEGMENT(entity)+1)
            {
                WhiteDragon_SpawnDeathPiece(entity,entity->posX.whole,entity->posY.whole,3);
                EntityDelete(entity);
                return;
            }
        }
        if (STATE_WORD(owner)==2 && SEGMENT(entity)<=5 &&
           owner->timer<(10-SEGMENT(entity))*8 && !(owner->timer & 7))
            WhiteDragon_SpawnSpark(entity,entity->posX.whole,entity->posY.whole,entity->spriteLayer-1);
        if (!Enemy_IsStunnedNoFlash(owner))
            Anim_Update(entity);
    }
    else
        EntityDelete(entity);
}
