/* Lightning Doll helpers reconstructed from the original assembly. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern void LightningDoll_SpawnDeathParts(struct EwramData_EntityData *entity);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);
extern void LightningDoll_UpdateDeath(struct EwramData_EntityData *entity);
extern void Sound_StopTracked(s32 id);
extern void LightningDoll_SpawnPlayerEffect(void);

/**
 * @brief Clear velocity and acceleration, snap to ground and enter state 1 with randomized movement and floor-wait countdowns.
 */
void LightningDoll_ResetActive(struct EwramData_EntityData *entity)
{
    s32 zero = 0;
    entity->accelX.whole = zero;
    entity->velX.whole = zero;
    entity->accelY.whole = zero;
    entity->velY.whole = zero;
    Entity_SnapToGround(entity);
    entity->state = 1;
    entity->stage = zero;
    entity->subState = zero;
    entity->unk_4F8.part16.unk_4F8 = RandomNumberGenerator() % 0x20 + 0x10;
    entity->unk_4FC.part16.unk_4FC = RandomNumberGenerator() % 0x400 + 0x200;
    entity->unk_4F8.part16.unk_4FA = zero;
}

/**
 * @brief Stop sound 0x75, spawn body parts and drop rewards once; request deletion when the live-part count reaches zero.
 */
void LightningDoll_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            Sound_StopTracked(0x75);
            PlaySong(0x6E);
            LightningDoll_SpawnDeathParts(entity);
            Enemy_DropRewards(entity);
            entity->palette = entity->unk_510.part8.basePalette;
            entity->state = 0xFF;
            entity->subState++;
        case 1:
            if (entity->unk_500.part8.unk_500 == 0)
                entity->deleteRequested = 1;
            break;
    }
}

/**
 * @brief Submit the attack contact record; during animation 3 or 4 also create the electrical effect at the player.
 */
void LightningDoll_OnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage effect;
    effect.attack = entity->attack;
    effect.attributes = 0;
    effect.unk_4 = 0;
    effect.reaction = 0;
    effect.sourceX = entity->posX.whole;
    effect.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&effect);
    if ((u8)(entity->animScriptIndex - 3) <= 1)
        LightningDoll_SpawnPlayerEffect();
}
