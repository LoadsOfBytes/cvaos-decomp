/* Mandragora death waits for its linked part and pending effects. */
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern void Mandragora_SpawnDeathParts(struct EwramData_EntityData *);
extern void Mandragora_RequestLinkedDeletion(struct EwramData_EntityData *);
#define FLAGS(e) (e->ext.mandragora.flags)
/**
 * @brief Death state (0xF0): hide, spawn five parts and wait for their live
 * count plus linked-part/palette state before requesting removal
 * of all three linked entities.
 */
void Mandragora_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part=&gEwramData->entityData[entity->ext.mandragora.linkIndex];
    switch(entity->subState)
    {
        case 0:
            entity->hidden=1;
            entity->palette=entity->unk_510.part8.basePalette;
            Mandragora_SpawnDeathParts(entity);
            entity->subState++;
            if(FLAGS(entity)&2)
                goto wait_part;
            PlaySong(0x6B);
        case 1:
            if(!(FLAGS(entity)&2))
            {
                if(entity->ext.mandragoraDeath.parts==0)
                    Mandragora_RequestLinkedDeletion(entity);
            }
            else
            {
wait_part:
                if(part->hidden && !(FLAGS(entity)&0x10) && entity->ext.mandragoraDeath.parts==0)
                    Mandragora_RequestLinkedDeletion(entity);
            }
            break;
    }
}
