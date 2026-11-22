/* Lightning Doll helpers reconstructed from the original assembly. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 Enemy_IsStunned(struct EwramData_EntityData *entity);
extern void Hit_CheckWithPlayer(void *buffer, struct EwramData_EntityData *entity);
extern void LightningDoll_UpdateState2Animation(struct EwramData_EntityData *entity);
extern void LightningDoll_UpdateActive(struct EwramData_EntityData *entity);
extern void LightningDoll_UpdateDeath(struct EwramData_EntityData *entity);
extern void Sound_StopTracked(s32 id);
extern void HitBox_SetAttackRaw(struct EwramData_EntityData *entity, s32 flags);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);

void EnemyLightningDollUpdate(struct EwramData_EntityData *entity)
{
    u8 buffer[0x10];
    if (entity->state == 2)
    {
        LightningDoll_UpdateState2Animation(entity);
        return;
    }
    Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity) && entity->state != 0xFF)
    {
        Sound_StopTracked(0x75);
        Hit_CheckWithPlayer(buffer, entity);
        return;
    }
    switch (entity->state)
    {
        case 1:
            LightningDoll_UpdateActive(entity);
            break;
        case 0xFF:
            LightningDoll_UpdateDeath(entity);
            break;
    }
    if (entity->state != 0xFF)
    {
        u8 frame = entity->animFrame;
        if (Anim_Update(entity) && frame != entity->animFrame)
        {
            if (entity->animScriptIndex == 4)
                HitBox_SetAttackRaw(entity, 0x1C1EE0F2);
            else
                Entity_UpdateBoxes(entity);
        }
        Hit_CheckWithPlayer(buffer, entity);
    }
}
