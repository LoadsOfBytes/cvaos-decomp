/* Linked part mirrors its parent, fades, and coordinates the two limb sounds. */
#include "code_080D73B8.h"
#include "graham_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 GrahamPart_UpdatePaletteSequence(struct EwramData_EntityData *,s32);
/* The original reads the adjacent mode/substate bytes as one halfword. */
#define STATE_WORD(e) (*(u16 *)&(e)->state)
/**
 * @brief Mirror body flags, run its palette sequence and coordinate
 * sounds from both side parts using combined state/substate reads.
 * Enter the fade state when owner defeat flags are set.
 */
void GrahamBodyOverlay_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    u32 mode=entity->state;
    if(mode==0xD0)
    { Anim_Update(entity); return; }
    if(owner)
    {
        s32 zero;
        u16 flags;
        if(owner->unk_518.part8.enemyId!=entity->unk_518.part8.enemyId)
        { entity->deleteRequested=1; return; }
        zero=0;
        entity->ext.graham.flags=owner->ext.graham.flags;
        if((entity->ext.graham.flags & 0x10) && mode<=0xDF)
        {
            entity->enemyFlags|=2;
            entity->state=0xE0;
            entity->stage=zero;
            entity->subState=zero;
        }
    }
    {
        s32 state=entity->state;
        switch(state)
        {
            case 0:
                if(GrahamPart_UpdatePaletteSequence(entity,0))
                {
                    entity->state=1;
                    entity->stage=state;
                    entity->subState=state;
                }
                break;
            case 1:
                if(entity->ext.graham.flags & 0x10)
                {
                    entity->state=0xE0;
                    entity->stage=0;
                    entity->subState=0;
                }
                else
                    Anim_Update(entity);
                break;
            case 0xE0:
                if(entity->ext.graham.flags & 0x20)
                    entity->objMode=1;
                return;
        }
    }
    {
        struct EwramData_EntityData *first=(void *)entity->ext.graham.sidePart0;
        struct EwramData_EntityData *second;
        if(first->stage==8)
        {
            u32 mask=0xFFFF;
            u32 firstState=(u16)(STATE_WORD(first)&mask);
            if(firstState==0x102)
            {
                second=(void *)GRAHAM_SIDE_PART_1(entity);
                if(second->stage==8 && (mask &= STATE_WORD(second))==firstState)
                    PlaySong(0x10E);
                else
                    PlaySong(0x10F);
                return;
            }
        }
        second=(void *)GRAHAM_SIDE_PART_1(entity);
        if(second->stage==8 && STATE_WORD(second)==0x102)
            PlaySong(0x10F);
    }
}
