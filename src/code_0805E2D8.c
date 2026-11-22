#include "code_08009A0.h"
#include "event_scene03.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern void Effect_SpawnAt(s32, s32, s32);
extern void EventScene03_SpawnPartDebris(struct EwramData_EntityData *owner);

/**
 * @brief 5E2D8 | Event part like EventScene03MainPart_Update: bobs (sine on unk_50A) while
 * in states 0-2, advances on owner flags 8 / 2, in state 3 flies along a
 * sine path (velX doubled, velY / 3) facing its direction; state 4 blinks
 * the palette (palette) and bursts at random (Rng % 7 == attack) with
 * sound 0x6B and debris via EventScene03_SpawnPartDebris. Owner flag 4 makes it vanish.
 */
void EventScene03FlyingPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 wave;

    if ((u16)EVENT03_PROGRESS(owner) & 4)
    {
        entity->state = 5;
        entity->stage = 0;
        entity->subState = 0;
    }
    if (entity->state <= 2)
    {
        wave = Sin(EVENT03_PART_WAVE(entity) + 0x4000) / 4;
        EVENT03_PART_WAVE(entity) += 0x200;
        entity->posY.whole += wave;
        entity->velY.whole = wave + entity->accelY.whole;
    }
    switch (entity->state)
    {
    case 0:
        if ((u16)EVENT03_PROGRESS(owner) & 8)
        {
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 1:
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
        break;
    case 2:
        if ((u16)EVENT03_PROGRESS(owner) & 2)
        {
            entity->state = 4;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    case 3:
        if ((u16)EVENT03_PROGRESS(owner) & 2)
        {
            entity->state = 4;
            entity->stage = 0;
            entity->subState = 0;
        }
        else
        {
            u16 angle = (EVENT03_PART_WAVE(entity) << 23) >> 16;
            s32 vx;
            s32 vy;
            entity->velX.whole = Sin(angle + 0x4000) >> 8 << 8;
            entity->velY.whole = Sin(angle) >> 8 << 8;
            vx = entity->velX.whole << 1;
            entity->velX.whole = vx;
            vy = entity->velY.whole / 3;
            entity->velY.whole = vy;
            entity->posX.whole += vx;
            entity->posY.whole += vy;
            EVENT03_PART_WAVE(entity)++;
            entity->flipX = vx >= 0;
        }
        break;
    case 4:
        switch (entity->subState)
        {
        case 0:
            entity->timer = 0x20;
            entity->subState++;
            entity->stage = 0;
            break;
        case 1:
            if (entity->timer & 1)
                entity->palette = 4;
            else
                entity->palette = entity->unk_510.part8.basePalette;
            if (--entity->timer == 0xFF)
            {
                entity->subState++;
                entity->stage = 0;
                entity->palette = entity->unk_510.part8.basePalette;
                entity->timer = 0x10;
            }
            break;
        case 2:
            if (--entity->timer == 0xFF)
            {
                entity->subState++;
                entity->stage = 0;
            }
            break;
        case 3:
            if (RandomNumberGenerator() % 7 == entity->attack)
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
        }
        break;
    case 5:
        entity->deleteRequested = 1;
        break;
    }
    Anim_Update(entity);
}
