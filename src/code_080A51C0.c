/* Chaos second form: ring shot update (51C0). */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

struct Chaos2Data {
    s32 graphics;
    s32 palette;
    s32 palette2;
};
/* frame and mirror bits of a shot per twelfth of a turn */
struct ShotFrame {
    u16 frame;
    u16 mirror;
};

extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void EntityMove(struct EwramData_EntityData *);

/* Shot: wait with the core, then fly; vanish off screen, after 240
 * frames or when the core is gone or dying. */
void Chaos2Shot_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct EwramData_EntityData *core = entity->unk_4F4.unk_4F4_entity;

    if (entity->state == 0)
    {
        if (--entity->timer <= 1)
        {
            entity->state++;
            PlaySong(0x146);
        }
        if (core != NULL && core->updateFunc != NULL
            && core->unk_518.part8.enemyId == entity->unk_518.part8.enemyId
            && core->state != 3)
            return;
        goto delete;
    }
    else if (entity->state == 1)
    {
        EntityMove(entity);
        Hit_CheckWithPlayer(collision, entity);
        if (EntityIsOutsideScreen(entity, 1) || entity->timer++ > 0xEF)
        {
        delete:
            EntityDelete(entity);
            return;
        }
        if (core == NULL || core->updateFunc == NULL
            || core->unk_518.part8.enemyId != entity->unk_518.part8.enemyId
            || core->state == 3)
            EntityDelete(entity);
    }
}
