#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Entity_IsTargetRight(struct EwramData_EntityData *entity);
extern s32 Entity_GetVisibilityPattern(s32 step);

#define FRAME(e) (*(u16 *)&(e)->animStep)

/**
 * @brief A04B0 | Enemy sub-state (file 0x080A02AC): animation 2 facing the
 * player, layer 0x11, 16 px higher; song 0x85 on frame 4, then animation 3
 * and 40 frames of blinking (Entity_GetVisibilityPattern); then back to state 0 hidden
 * behind (layer 0x1D) with a random 64..127 timer in unk_4FE and unk_4FC =
 * 1.
 */
void KyomaDemon_UpdateAttack(struct EwramData_EntityData *entity)
{
    u32 r;

    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->flipX = Entity_IsTargetRight(entity);
        entity->ext.kyomaDemon.hidden = 0;
        entity->scrollWithBg = 1;
        entity->subState++;
        entity->spriteLayer = 0x11;
        entity->posY.whole -= 0x100000;
        break;
    case 1:
        if (FRAME(entity) == 4)
            PlaySong(0x85);
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 3, 0, 0);
            entity->subState++;
            entity->timer = 0x28;
        }
        break;
    case 2:
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0)
        {
            entity->state = 0;
            entity->subState = 0;
            entity->scrollWithBg = 0;
            entity->hidden = 0;
            r = (RandomNumberGenerator() & 0x3F) + 0x40;
            entity->ext.kyomaDemon.lurkTimer = r;
            entity->ext.kyomaDemon.hidden = 1;
            entity->spriteLayer = 0x1D;
        }
        break;
    }
}
