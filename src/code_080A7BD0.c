#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void SkyFish_UpdateFly(struct EwramData_EntityData *);
extern void SkyFish_UpdateDeath(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);

void EnemySkyFishUpdate(struct EwramData_EntityData *entity)
{
    /* Original reserves 0x30 bytes for this scratch area. */
    u32 box[12];
    u8 frame;
    if (entity->state != 1)
    {
        if (Entity_IsInRect(entity, -16, 0, 0x110, 0xB0)) Player_TrackEntity(entity);
    }
    if (Enemy_IsStunned(entity))
    {
        if (entity->state != 1 && entity->ext.skyFish.darting == 0)
            Hit_CheckWithPlayer(box, entity);
        return;
    }
    switch (entity->state)
    {
    case 0: SkyFish_UpdateFly(entity); break;
    case 1: SkyFish_UpdateDeath(entity); break;
    case 2: Anim_Update(entity); return;
    }
    frame = entity->animFrame;
    Anim_Update(entity);
    if (frame != entity->animFrame) Entity_UpdateBoxes(entity);
    if (entity->state != 1 && entity->ext.skyFish.darting == 0)
        Hit_CheckWithPlayer(box, entity);
}
