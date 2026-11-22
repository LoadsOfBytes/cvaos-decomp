/* Linked combat body mirrors event flags and transitions to its fade state. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 GrahamPart_UpdatePaletteSequence(struct EwramData_EntityData *,s32);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
/**
 * @brief Mirror root identity/event flags, run the palette entry sequence
 * and animate/check contact while active. On root defeat flags enter
 * the fade state or request deletion. Bestiary only animates.
 */
void GrahamCombatBody_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[entity->unk_4F4.part8.unk_4F5];
    if(entity->state==0xD0)
    {
        Anim_Update(entity);
        return;
    }
    if(owner)
    {
        if(owner->unk_518.part8.enemyId!=entity->unk_518.part8.enemyId)
            entity->deleteRequested=1;
        entity->unk_4F8.part16.unk_4FA=owner->unk_4F8.part16.unk_4FA;
        if(entity->unk_4F8.part16.unk_4FA & 0x40)
            entity->deleteRequested=1;
        if(entity->deleteRequested)
            return;
    }
    {
        s32 state=entity->state;
        switch(state)
        {
            case 0:
                if(GrahamPart_UpdatePaletteSequence(entity,1))
                {
                    entity->state=1;
                    entity->stage=state;
                    entity->subState=state;
                }
                break;
            case 1:
                if(entity->unk_4F8.part16.unk_4FA & 0x10)
                {
                    entity->state=0xE0;
                    entity->stage=0;
                    entity->subState=0;
                }
                Anim_Update(entity);
                Enemy_CheckHitPlayer(entity);
                break;
            case 0xE0:
                if(entity->unk_4F8.part16.unk_4FA & 0x20)
                    entity->objMode=1;
                break;
        }
    }
}
