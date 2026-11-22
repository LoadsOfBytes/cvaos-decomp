/* Manticore_UpdateAttack: enemy attack sequence that spawns two projectiles on frame 14. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

void PlaySong(u32 song);
extern void Manticore_SpawnShot(struct EwramData_EntityData *entity);
extern void Manticore_SpawnShotFlash(struct EwramData_EntityData *entity);

#define FRAME_PAIR(e) (*(u16 *)&(e)->animStep)

struct FramePairBits { u16 value:16; };
struct FramePairBits32 { u32 value:16; };

/**
 * @brief Attack state machine (state 1): animations 2 and 3, then the breath
 * (Manticore_SpawnShot + Manticore_SpawnShotFlash, sounds 0x7A / 0x77);
 * back to state 0.
 */
void Manticore_UpdateAttack(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->unk_4F8.unk_4F8_entity;

    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->subState++;
        parent->unk_500.part8.unk_503 = 0;
        break;
    case 1:
        if (entity->animStep == 0)
            break;
        entity->animFlags |= 2;
        entity->subState++;
        break;
    case 2:
        if (parent->unk_500.part8.unk_503 <= 2)
            break;
        entity->animFlags &= ~2;
        entity->subState++;
        break;
    case 3:
        if (parent->unk_500.part8.unk_503 <= 4)
            break;
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->subState++;
        entity->animDone = 0;
        entity->timer = 0;
        break;
    case 4:
        if (entity->timer++ <= 0x1D)
            break;
        entity->state = 0;
        entity->subState = 2;
        Anim_StartIndexed(entity, 0, 0, 1);
        break;
    case 10:
        Anim_StartIndexed(entity, 4, 0, 0);
        entity->subState++;
        parent->unk_500.part8.unk_503 = 0;
        entity->timer = 0;
        entity->unk_508.part8.unk_508 = 0;
        if ((RandomNumberGenerator() & 0xFF) <= 0x7F)
            entity->unk_508.part8.unk_509 = 1;
        else
            entity->unk_508.part8.unk_509 = (RandomNumberGenerator() % 2) + 2;
        break;
    case 11:
    {
        u32 m = 0xFFFF;
        if ((FRAME_PAIR(entity) & m) == 0xE)
        {
            PlaySong(0x7A);
            Manticore_SpawnShot(entity);
            Manticore_SpawnShotFlash(entity);
            PlaySong(0x77);
        }
        m &= FRAME_PAIR(entity);
        if (m == 0x14)
        {
            if ((s8)++entity->unk_508.part8.unk_508 < (s8)entity->unk_508.part8.unk_509)
            {
                Anim_StartIndexed(entity, 4, 0, 0);
                entity->animTimer = 0;
                entity->animStep = 2;
            }
        }
        if (entity->animDone)
        {
            entity->subState++;
            entity->timer = 0;
        }
        break;
    }
    case 12:
        if (entity->timer++ <= 3)
            break;
        entity->state = 0;
        entity->subState = 2;
        Anim_StartIndexed(entity, 0, 0, 1);
        break;
    }
}
