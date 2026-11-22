/* Altair feather motion. Local register bindings preserve phase accumulation
 * and threshold selection in agbcc; they do not prove original source spelling. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/entity_template.h"

/**
 * @brief Initialize a feather near the owner, drift downward with sine
 * wobble, then request deletion at the phase threshold. Preserve the
 * pinned-register phase and threshold calculation.
 */
void AltairFeather_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->unk_4F4.unk_4F4_entity;
    s32 phase;
    register u32 limit asm("r0"); /* FAKEMATCH: pinned register */
    switch (entity->state)
    {
    case 0:
    case 1:
        entity->spriteLayer = 2;
        {
            u32 random = RandomNumberGenerator();
            u16 *position = &parent->posX.part16.integer;
            s32 mask = 0x1F;
            mask &= random;
            entity->posX.part16.integer = mask + 0xFFF0 + *position;
        }
        {
            u32 random = RandomNumberGenerator();
            u16 *position = (u16 *)&parent->posY.part16.integer;
            s32 mask = 0xF;
            mask &= random;
            {
                s32 offset = mask + 0xFFF8;
                u16 value = *position;
                entity->posY.part16.integer = offset + value;
            }
        }
        entity->velY.whole = 0x1000;
        entity->state++;
        entity->timer = 0;
        {
            u32 random = RandomNumberGenerator();
            s32 mask = 0x3FFF;
            mask &= random;
            entity->unk_508.part16.unk_508 = mask - 0x2000;
        }
        entity->unk_508.part16.unk_50A = 0xD00;
        {
            u32 random = RandomNumberGenerator();
            s32 mask = 0x7F;
            mask &= random;
            entity->unk_4F8.part16.unk_4F8 = mask + 0x40;
        }
        entity->scrollWithBg = 1;
        break;
    case 2:
        entity->posX.whole += Sin((u16)entity->unk_4F8.part16.unk_4FA) / 4;
        entity->posY.whole += entity->velY.whole;
        {
            u32 value = (u16)entity->unk_4F8.part16.unk_4F8;
            {
                u32 increment = (u16)entity->unk_4F8.part16.unk_4FA;
                value += increment;
            }
            entity->unk_4F8.part16.unk_4FA = value;
            phase = value << 16;
        }
        if (phase < 0)
            limit = 0x9FFF0000;
        else
            limit = 0x1FFF0000;
        if ((u32)phase > limit)
            entity->state++;
        break;
    case 3:
        entity->deleteRequested = 1;
        break;
    }
}
