#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern s32 Entity_TryWalk(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void Effect_SpawnAt(s32, s32, s32);
extern void EventScene03_SpawnTargetOrb(s32, s32, s32, s32, s32);

/* Event part: wait for its owner, rise, blink, then emit debris. */
void EventScene03RisingPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[entity->unk_4F4.part8.unk_4F5];
    if ((u16)owner->unk_504.part16.unk_504 & 4)
    {
        entity->state = 4;
        entity->stage = 0;
        entity->subState = 0;
    }
    switch (entity->state)
    {
    case 0:
        if ((u16)owner->unk_504.part16.unk_504 & 8)
        {
            if (RandomNumberGenerator() % 7 == entity->attack)
            {
                if ((u8)(entity->unk_4F4.part8.unk_4F7 & 2))
                {
                    entity->state = 2;
                    entity->stage = 0;
                    entity->subState = 0;
                }
                else
                {
                    entity->state = 1;
                    entity->stage = 0;
                    entity->subState = 0;
                }
            }
        }
        break;
    case 1:
        switch (entity->subState)
        {
        case 0:
            if (RandomNumberGenerator() % 7 == entity->attack)
            {
                entity->timer = 16;
                entity->subState++;
                entity->stage = 0;
            }
            break;
        case 1:
            if (RandomNumberGenerator() % 3 == 0)
            {
                if (--entity->timer == 255)
                {
                    entity->state = 2;
                    entity->stage = 0;
                    entity->subState = 0;
                }
                else entity->posY.whole -= 0x8000;
            }
            break;
        case 2: break;
        }
        break;
    case 2:
        if (entity->unk_4F4.part8.unk_4F7 & 8)
        {
            switch (entity->subState)
            {
            case 0:
                if (entity->animScriptIndex != 1) Anim_StartIndexed(entity, 1, 0, 1);
                entity->timer = 160;
                entity->subState++;
                entity->stage = 0;
                /* fall through */
            case 1:
                if (!(RandomNumberGenerator() & 1) && entity->timer)
                {
                    if (!(Entity_TryWalk(entity, 0x4000, 0x200000, 0x20000, 1) & 4))
                        entity->posY.whole += entity->velY.whole;
                    entity->timer--;
                }
                break;
            }
        }
        else if (entity->unk_4F4.part8.unk_4F7 & 16)
        {
            if (entity->subState == 0)
            {
                if (entity->animScriptIndex != 2) Anim_StartIndexed(entity, 2, 0, 0);
                entity->subState++;
            }
        }
        if ((u16)owner->unk_504.part16.unk_504 & 2)
        {
            entity->state = 3;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 3:
        switch (entity->subState)
        {
        case 0:
            entity->timer = 32;
            entity->subState++;
            entity->stage = 0;
            break;
        case 1:
            if (entity->timer & 1) entity->palette = 4;
            else entity->palette = entity->unk_510.part8.basePalette;
            if (--entity->timer == 255)
            {
                entity->subState++;
                entity->stage = 0;
                entity->palette = entity->unk_510.part8.basePalette;
            }
            break;
        case 2:
            if (RandomNumberGenerator() % 7 == entity->attack)
            {
                entity->subState++;
                entity->stage = 0;
            }
            break;
        case 3:
            if (RandomNumberGenerator() % 7 == entity->attack)
            {
                if (Entity_IsInRect(entity, -128, -64, 496, 288))
                {
                    PlaySong(0x6B);
                    Effect_SpawnAt(entity->posX.whole, entity->posY.whole, entity->spriteLayer - 1);
                }
                EventScene03_SpawnTargetOrb((s16)entity->posX.part16.integer, entity->posY.part16.integer - 32, 0, 0, owner->unk_4FC.whole);
                entity->state = 4;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
        }
        break;
    case 4:
        entity->deleteRequested = 1;
        break;
    }
    Anim_Update(entity);
}
