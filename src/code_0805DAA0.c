#include "code_08009A0.h"
#include "event_scene03.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern s32 Entity_TryWalk(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void Effect_SpawnAt(s32, s32, s32);
extern void EventScene03_SpawnPartDebris(struct EwramData_EntityData *owner);

/**
 * @brief 5DAA0 | Event part like EventScene03RisingPart_Update: bob up and down after the
 * owner sets flag 8, then (flag 0x10) wait 0x60 frames and walk left until
 * room x 0xCA; owner flag 4 makes it vanish (state 4 bursts with sound 0x6B
 * and debris via EventScene03_SpawnPartDebris).
 */
void EventScene03MainPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 wave;

    if ((u16)owner->ext.event03.progress & 4)
    {
        entity->state = 5;
        entity->stage = 0;
        entity->subState = 0;
    }
    switch (entity->state)
    {
    case 0:
        if ((u16)owner->ext.event03.progress & 8)
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 1:
        wave = Sin(entity->ext.event03Part.wave + 0x4000) / 4;
        entity->ext.event03Part.wave += 0x200;
        entity->posY.whole += wave;
        entity->velY.whole = wave + entity->accelY.whole;
        if ((u16)owner->ext.event03.progress & 0x10)
        {
            entity->state = 2;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 2:
        switch (entity->subState)
        {
        case 0:
            switch (entity->stage)
            {
            case 0:
                if (entity->animScriptIndex != 5)
                    Anim_StartIndexed(entity, 5, 0, 1);
                entity->timer = 0x60;
                entity->stage++;
                break;
            case 1:
                if (--entity->timer == 0xFF)
                {
                    entity->subState++;
                    entity->stage = 0;
                }
                break;
            }
            break;
        case 1:
            if ((s16)entity->posX.part16.integer > 0xCA - (s16)(gEwramData->bgInfo + 1)->xPos.part16.integer)
            {
                if (!(Entity_TryWalk(entity, 0x30000, 0x200000, 0x14000, 1) & 4))
                    entity->posY.whole += entity->velY.whole;
                entity->posX.whole += entity->velX.whole;
            }
            else
            {
                if (entity->animScriptIndex != 2)
                    Anim_StartIndexed(entity, 2, 0, 1);
                entity->state = 3;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
        }
        break;
    case 4:
        if (entity->subState == 0)
        {
            if (Entity_IsInRect(entity, -0x80, -0x40, 0x1F0, 0x120))
            {
                PlaySong(0x6B);
                Effect_SpawnAt(entity->posX.whole, entity->posY.whole, entity->spriteLayer - 1);
                EventScene03_SpawnPartDebris(entity);
            }
            entity->state = 5;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 5:
        entity->deleteRequested = 1;
        break;
    }
    Anim_Update(entity);
}
