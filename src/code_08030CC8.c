#include "code_08009A0.h"
#include "player_entity.h"
#include "code_080D73B8.h"
extern s32 BlueSoul_ShouldEnd(struct EwramData_EntityData *);
extern void BlueSoul_ReleaseActiveResources(struct EwramData_EntityData *);

void SkillMedusaHeadUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->ext.blueSoul.owner;
    s32 done;
    s32 velocity;
    if (entity->state == 0)
    {
        if ((parent->ext.player.actionFlags & 0x01008082) != 2) goto restore;
        velocity = parent->velY.whole;
        entity->velY.whole = velocity;
        entity->accelY.whole = parent->accelY.whole;
        if (velocity < 0) entity->velY.whole = 0;
        entity->accelY.whole = 0x2800;
        PlaySong(0x8C);
        entity->state = 1;
    }
    done = BlueSoul_ShouldEnd(entity);
    if (done)
    {
        parent->velY.whole = entity->velY.whole;
        parent->accelY.whole = entity->accelY.whole;
        goto remove;
    }
    if ((parent->ext.player.actionFlags & 0x01008082) != 2)
    {
        parent->velY.whole = entity->velY.whole;
        parent->accelY.whole = entity->accelY.whole;
restore:
        BlueSoul_ReleaseActiveResources(entity);
remove:
        EntityDelete(entity);
        return;
    }
    gEwramData->playerState.actionLockFlags |= 0x20;
    gEwramData->playerState.player->velY.whole = done;
    gEwramData->playerState.player->accelY.whole = done;
}
