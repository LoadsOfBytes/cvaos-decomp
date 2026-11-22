/* SkillGhostUse; structure as
 * SkillHarpyUse (src/code_0802CC84.c). */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08040A38.h"
extern void SkillGhostSpiritUpdate(struct EwramData_EntityData *);

/**
 * @brief 2A8D4 | Ghost red soul: position at the caster (Entity_PlaceRelativeTo, 0
 * / -0x18), Hit_SetTimerOverride(1, 15), song 0xD6; spawn two SkillGhostSpiritUpdate ghosts
 * (RedSoul_CreateEntity) offset by sine/cosine << 4 around it, at angles -0x7800 and -0x1800
 * plus a random 0..0xFFF (stored in unk_510); then free the red soul slot.
 */
void SkillGhostUse(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->unk_4F8.unk_4F8_entity;
    s32 i;
    s32 angle;

    entity->roomActive = 1;
    switch (entity->state)
    {
    case 0:
        Entity_PlaceRelativeTo(entity, parent, 0, -0x18);
        entity->scrollWithBg = 1;
        Hit_SetTimerOverride(1, 15);
        gEwramData->playerState.nbrActiveRedSoulEntities++;
        PlaySong(0xD6);
        entity->state = 1;
    case 1:
        for (i = 0, angle = 0; i <= 1; angle += 0x6000, i++)
        {
            struct EwramData_EntityData *child = RedSoul_CreateEntity(entity, (u32 *)SkillGhostSpiritUpdate);
            if (child == NULL)
                break;
            {
                s32 offset = (s32)(RandomNumberGenerator() % 0x1000) - 0x7800;
                child->unk_510.part16.unk_510 = angle + offset;
            }
            child->posX.whole = entity->posX.whole + (Sin(child->unk_510.part16.unk_510 + 0x4000) << 4);
            child->posY.whole = entity->posY.whole + (Sin(child->unk_510.part16.unk_510) << 4);
        }
        entity->state = 2;
        break;
    case 2:
        gEwramData->playerState.activeRedSoulEntities[entity->unk_4FC.part16.unk_4FE[0]] = NULL;
        gEwramData->playerState.nbrActiveRedSoulEntities--;
        EntityDelete(entity);
        break;
    }
}
