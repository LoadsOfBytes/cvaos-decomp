#include "code_08009A0.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern void MinotaurAxe_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);

/* Collision object attached to a rotating parent, at a radius of 16 pixels. */
void MinotaurAxeHitBox_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    if (entity->state != 0 && *(u8 *)&parent->ext.raw.unk_4F4 != ENTITY_PARENT_INDEX(entity))
    {
        EntityDelete(entity);
        return;
    }
    switch (entity->state)
    {
    case 0:
        Hit_Enable(entity, 8, MinotaurAxe_DamagePlayerOnContact, 0);
        HitBox_SetAttackRaw(entity, 0x1414F6F6);
        entity->hit.flags = (entity->hit.flags & ~2) | 4;
        entity->state++;
        /* fall through */
    case 1:
        {
            s32 angle = parent->ext.minotaurAxe.angle;
            s32 yAngle = angle - 0x4000;
            s32 dx = Sin(angle) << 4;
            s32 dy = Sin(yAngle) << 4;
            entity->posX.whole = parent->posX.whole + dx;
            entity->posY.whole = parent->posY.whole + dy;
            if (parent->state == 1)
                Hit_CheckWithPlayer(collision, entity);
        }
        break;
    }
}
