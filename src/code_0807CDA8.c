/* Advance a six-step attack, turn at its midpoint and pause between steps. */
#include "code/code_08039340.h"
#include "code_080D73B8.h"
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
static inline s32 Flipped(s32 facing) { return facing ^ 1; }
/**
 * @brief Root wave update: after each animation (sound 0x80) jump 40 px further,
 * turning once after the third; delete after six.
 */
void AluraUneRootWave_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    switch (entity->state)
    {
    case 0:
        if (entity->animStep == 0 && entity->animTimer == 1) PlaySong(0x80);
        if (entity->animDone)
        {
            entity->state++;
            if (++entity->ext.aluraUneRootWave.step > 5) { entity->deleteRequested = 1; goto finished; }
            if (entity->ext.aluraUneRootWave.step == 3) entity->flipX ^= 1;
            else if (entity->flipX) entity->posX.part16.integer += 40;
            else entity->posX.part16.integer -= 40;
            entity->hidden = 1;
            entity->timer = 0;
            entity->hit.flags |= 2;
            if (!parent || !parent->updateFunc || parent->enemyId != 0x4D || parent->state == 2)
            {
                entity->deleteRequested = 1;
                goto finished;
            }
        }
        else
        {
            Entity_UpdateBoxes(entity);
            Hit_CheckWithPlayer(collision, entity);
        }
        break;
    case 1:
        if (entity->timer++ > 5)
        {
            Anim_StartIndexed(entity, 9, 0, 0);
            entity->hidden = 0;
            entity->state = 0;
            entity->hit.flags &= ~2;
        }
        if (!parent || !parent->updateFunc || parent->enemyId != 0x4D || parent->state == 2)
        {
            entity->deleteRequested = 1;
            goto finished;
        }
        break;
    }
    Anim_Update(entity);
finished:
    return;
}
