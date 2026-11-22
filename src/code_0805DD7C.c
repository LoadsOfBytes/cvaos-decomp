#include "code_08009A0.h"
#include "event_scene03.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern s32 Entity_TryWalk(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void Effect_SpawnAt(s32, s32, s32);
extern void EventScene03_SpawnTargetOrb(s32, s32, s32, s32, s32);
extern struct EwramData_EntityData *Entity_CreateFromObjectTable(s32);
extern void EventScene03AttachedHelper_Update(struct EwramData_EntityData *);
extern void EventScene03_SpawnUnusedPartDebris(struct EwramData_EntityData *);

/* Event part (EventScene03UnusedPart_Update, cf. EventScene03RisingPart_Update): wait for its owner, rise,
 * shake (optionally spawning a EventScene03AttachedHelper_Update helper), blink, then break apart. */
void EventScene03UnusedPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    struct EwramData_EntityData *other = &gEwramData->entityData[EVENT03_PART_LINK_INDEX(entity)];
    struct EwramData_EntityData *child;
    if ((u16)EVENT03_PROGRESS(owner) & 4)
    {
        entity->state = 4;
        entity->stage = 0;
        entity->subState = 0;
    }
    switch (entity->state)
    {
    case 0:
        if ((u16)EVENT03_PROGRESS(owner) & 8)
        {
            if (RandomNumberGenerator() % 7 == entity->attack)
            {
                if ((u8)(EVENT03_PART_FLAGS(entity) & 2))
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
        if (EVENT03_PART_FLAGS(entity) & 8)
        {
            switch (entity->subState)
            {
            case 0:
                if (EVENT03_PART_FLAGS(entity) & 16)
                {
                    child = Entity_CreateFromObjectTable(9);
                    child->updateFunc = (u32 *)EventScene03AttachedHelper_Update;
                    ENTITY_PARENT_INDEX(child) = ENTITY_INDEX(entity);
                    child->posX.part16.integer = entity->posX.part16.integer;
                    child->posY.part16.integer = entity->posY.part16.integer;
                    Anim_StartIndexed(child, 8, 0, 1);
                    child->spriteLayer = entity->spriteLayer;
                    child->unk_518.part8.enemyId = entity->unk_518.part8.enemyId;
                    if (entity->animScriptIndex != 2) Anim_StartIndexed(entity, 2, 0, 1);
                }
                else
                {
                    if (entity->animScriptIndex != 1) Anim_StartIndexed(entity, 1, 0, 1);
                }
                entity->timer = 160;
                entity->subState++;
                entity->stage = 0;
                /* fall through */
            case 1:
                if (entity->timer && !(RandomNumberGenerator() & 1))
                {
                    if (!(Entity_TryWalk(entity, 0x4000, 0x200000, 0x20000, 1) & 4))
                        entity->posY.whole += entity->velY.whole;
                    entity->timer--;
                }
                break;
            }
        }
        if ((u16)EVENT03_PROGRESS(owner) & 2)
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
                    EventScene03_SpawnUnusedPartDebris(entity);
                }
                EventScene03_SpawnTargetOrb((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer - 32, 0, 0, owner->unk_4FC.whole);
                entity->state = 4;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
        }
        break;
    case 4:
        entity->deleteRequested = 1;
        other->deleteRequested = 1;
        break;
    }
    Anim_Update(entity);
}
