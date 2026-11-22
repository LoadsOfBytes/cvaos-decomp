/* Witch projectile and particle helpers. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void WitchOrbitParticle_Update(struct EwramData_EntityData *);
extern u8 sWitchGfx[], sWitchPalette[], sWitchSpriteSheet[];

/**
 * @brief Spawn up to two orbit particles at phases separated by 0x2000,
 * with the center and rotation direction mirrored by facing.
 */
void Witch_SpawnOrbitParticles(struct EwramData_EntityData *entity, u16 phase)
{
    s32 count = 2;
    struct EwramData_EntityData *child;
    for (;;)
    {
        child = EntityCreateInRange(0x49, 0xCF, (u32 *)WitchOrbitParticle_Update);
        if (child != NULL)
        {
            child->accelX.part16.integer = entity->posX.part16.integer;
            child->accelY.part16.integer = entity->posY.part16.integer - 0x10;
            child->unk_4F8.part16.unk_4FA = 0xFC00;
            child->flipX = entity->flipX;
            if (child->flipX)
            {
                child->unk_4F8.part16.unk_4FA = -(u16)child->unk_4F8.part16.unk_4FA;
                child->accelX.part16.integer += 0x20;
            }
            else child->accelX.part16.integer -= 0x20;
            child->unk_4F8.part16.unk_4F8 = phase;
            phase = (((u32)phase << 16) + 0x20000000) >> 16;
            if (--count == 0) break;
        }
        else break;
    }
}

/**
 * @brief Animate and move a spell trail particle, enable drawSkip after
 * the first countdown, then remove after the second. Retain state fallthrough.
 */
void WitchSpellTrailParticle_Update(struct EwramData_EntityData *entity)
{
    s32 timer;
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sWitchGfx, sWitchPalette, 0, sWitchSpriteSheet)) goto remove;
        Anim_StartIndexed(entity, 0xE, 0, 0);
        entity->spriteLayer = 3;
        entity->timer = 6;
        entity->state++;
        /* fall through */
    case 1:
        EntityAccelerateAndMove(entity, 0x80000);
        timer = entity->timer;
        if (timer != 0) goto countdown;
        entity->drawSkip = 1;
        entity->timer = 4;
        entity->state++;
        /* fall through */
    case 2:
        EntityAccelerateAndMove(entity, 0x80000);
        timer = entity->timer;
        if (timer == 0)
        {
remove:
            EntityDelete(entity);
            return;
        }
countdown:
        entity->timer = timer - 1;
        break;
    }
    Anim_Update(entity);
}

