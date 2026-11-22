#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Entity_IsTargetRight(struct EwramData_EntityData *entity);

/**
 * @brief 8F6E8 | Enemy attack sub-state (as SkeletonKnight_UpdateAttack, file
 * 0x0808F364): animation 2 facing the player, after 38 frames animation 3
 * and with 1/2 chance animation 4 (song 0x85 on frame 11, 30 frames) then
 * 20 frames of animation 3, or else animation 6 repeated four times (song
 * 0x85 on frame 11, 12 frames each); then back to state 0.
 */
void DeadCrusader_UpdateAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        entity->flipX = Entity_IsTargetRight(entity);
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->subState++;
        entity->timer = 0;
        break;
    case 1:
        if (entity->timer++ > 0x25)
        {
            Anim_StartIndexed(entity, 3, 0, 0);
            entity->timer = 0;
            if ((RandomNumberGenerator() & 0x7F) <= 0x3F)
                entity->subState++;
            else
                entity->subState = 5;
        }
        break;
    case 2:
        if (entity->timer == 0xB)
            PlaySong(0x85);
        if (entity->timer++ > 0x1D)
        {
            Anim_StartIndexed(entity, 4, 0, 0);
            entity->subState++;
            entity->timer = 0;
        }
        break;
    case 3:
        if (entity->timer++ <= 0x13)
            break;
        goto done;
    case 4:
        break;
    case 5:
        if (entity->timer == 0xB)
            PlaySong(0x85);
        if (entity->timer++ > 0xB)
        {
            Anim_StartIndexed(entity, 6, 0, 0);
            entity->subState++;
            entity->stage = 0;
            entity->timer = 0;
        }
        break;
    case 6:
        if (entity->timer++ > 0xB)
        {
            if (entity->stage++ > 2)
            {
            done:
                Anim_StartIndexed(entity, 1, 0, 1);
                entity->state = 0;
                entity->subState = 1;
                entity->timer = 0;
                entity->velY.whole = 0x10000;
                entity->accelY.whole = 0x2800;
            }
            else
            {
                Anim_StartIndexed(entity, 6, 0, 0);
                entity->timer = 0;
            }
        }
        break;
    }
}
