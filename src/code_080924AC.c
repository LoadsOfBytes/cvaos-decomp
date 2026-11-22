/* ZombieOfficer_UpdateAttack. */
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/* animStep and animTimer read as one halfword (as in code_080756FC.c). */
#define FRAME_PAIR(e) (*(u16 *)&(e)->animStep)

/**
 * @brief 924AC | Attack: animation 1, sound 0x85 at frame 4; when it ends
 * go back to state 0 with animation 0 and a hop (velX +-0xA000 by facing,
 * velY 0x10000, accelY 0x2800).
 */
void ZombieOfficer_UpdateAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->subState++;
            break;
        case 1:
            if (FRAME_PAIR(entity) == 4)
                PlaySong(0x85);
            if (entity->animDone)
            {
                Anim_StartIndexed(entity, 0, 0, 1);
                entity->state = 0;
                entity->subState = 0;
                if (entity->flipX)
                    entity->velX.whole = 0xA000;
                else
                    entity->velX.whole = -0xA000;
                entity->velY.whole = 0x10000;
                entity->accelY.whole = 0x2800;
            }
            break;
    }
}
