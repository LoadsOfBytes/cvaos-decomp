/* Devil/Manticore: emit sparks every four frames, sound every eight frames. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "structs/ewram.h"
extern void SkillDevilManticoreSparkUpdate(struct EwramData_EntityData *);
void SkillDevilManticoreSparksUpdate(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->unk_4F8.unk_4F8_entity;
    entity->roomActive = 1;
    if (!(entity->timer & 7)) PlaySong(0x90);
    if (!(entity->timer & 3))
    {
        struct EwramData_EntityData *spark = BlueSoul_CreateEntity(entity, SkillDevilManticoreSparkUpdate);
        if (spark != NULL)
        {
            spark->posX.whole = owner->posX.whole;
            spark->posY.whole = owner->posY.whole;
            spark->flipX = owner->flipX;
        }
    }
    entity->timer++;
}
