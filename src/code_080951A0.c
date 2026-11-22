#include "code_08009A0.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

/*
 * Lilith's shot: flight angle (curves by 0x400 per frame during the first
 * frames) and its two afterimage entities (ended together with the shot).
 * Token-identical expansion.
 */
#define LILITH_SHOT_ANGLE(e) e->unk_500.whole
#define LILITH_SHOT_TRAIL_0(e) e->unk_4F8.unk_4F8_entity
#define LILITH_SHOT_TRAIL_1(e) e->unk_4FC.unk_4FC_entity
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Entity_EndIfActive(struct EwramData_EntityData *);
extern void EntitySetVelocityFromAngle(struct EwramData_EntityData *, s32, s32);
extern void EntityMove(struct EwramData_EntityData *);
void LilithShot_Update(struct EwramData_EntityData *entity)
{
    s32 buffer[4];
    switch (entity->state)
    {
    case 0:
        entity->timer++;
        if (entity->timer <= 14) LILITH_SHOT_ANGLE(entity) += 0x400;
        else if (entity->timer <= 28)
        {
            LILITH_SHOT_ANGLE(entity) -= 0x400;
            if (entity->timer > 27) entity->timer = 0;
        }
        EntitySetVelocityFromAngle(entity, LILITH_SHOT_ANGLE(entity), 0x10000);
        EntityMove(entity);
        Hit_CheckWithPlayer(buffer, entity);
        if (EntityIsOutsideScreen(entity, 1)) entity->deleteRequested = 1;
        break;
    case 1: entity->deleteRequested = 1; break;
    case 2: entity->deleteRequested = 1; break;
    }
    if (entity->deleteRequested)
    {
        if (LILITH_SHOT_TRAIL_0(entity) != NULL) Entity_EndIfActive(LILITH_SHOT_TRAIL_0(entity));
        if (LILITH_SHOT_TRAIL_1(entity) != NULL) Entity_EndIfActive(LILITH_SHOT_TRAIL_1(entity));
    }
}
