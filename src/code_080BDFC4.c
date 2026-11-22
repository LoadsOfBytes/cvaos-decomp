#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"

/*
 * Death reward effect: sprite rotation, affine scale (also its orbit radius,
 * grows to 0x10000 and shrinks back) and a frame counter whose low bit
 * alternates the effect. Token-identical expansion.
 */
#define REWARD_ROTATION(e) e->unk_4FC.part16.unk_4FC
#define REWARD_SCALE(e) e->unk_500.whole
#define REWARD_COUNTER(e) e->unk_4F8.part16.unk_4FA

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void SharedObjPalette_Release(void *palette, u8 variant);
extern void SoulPickup_SpawnFromEnemyDefinition(s32, s32, s32, s32);
extern void BossRecoveryPickup_Spawn(s32, s32, s32, s32, s32);

static inline s32 FixedToInt(s32 v)
{
    return (v < 0) ? -(-v >> 16) : (v >> 16);
}

/* Growing sparkle at the screen centre: grows, waits, drops an item
 * (BossRecoveryPickup_Spawn) and shrinks away, scattering debris every other frame. */
/**
 * @brief Grow and rotate an affine reward effect at room point (256,128).
 * After the countdown spawn the enemy-definition soul pickup and call
 * BossRecoveryPickup_Spawn, then shrink and release the affine slot; emit debris.
 */
void DeathRewardEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData **ew = &gEwramData; /* address loaded before the switch */
    u16 slot;

    switch (entity->state)
    {
    case 0:
        SharedObjPalette_Release((void *)0x0820BC1C, 2);
        if (!Entity_LoadGraphics(entity, (void *)0x08119498, (void *)0x0820BC1C, 3, (void *)0x08249C48))
            goto delete;
        slot = ObjAffine_Alloc(1);
        if ((s16)slot < 0)
            goto delete;
    {
        struct BgLayerState *camera = &(*ew)->bgInfo[1];
        entity->posX.part16.integer = 0x100 - camera->xPos.part16.integer;
        entity->posY.part16.integer = 0x80 - camera->yPos.part16.integer;
    }
        entity->affineIndex = slot;
        entity->affine = 1;
        entity->spriteLayer = 0x14;
        Anim_StartIndexed(entity, 0xF, 2, 0);
        Anim_Update(entity);
        PlaySong(0x120);
        entity->state = 1;
    case 1:
        if (REWARD_SCALE(entity) <= 0xFFFF)
        {
            REWARD_SCALE(entity) += 0x400;
            break;
        }
        entity->timer = 0x60;
        entity->state = 2;
    case 2:
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        SoulPickup_SpawnFromEnemyDefinition((s16)entity->posX.part16.integer, (s16)entity->posY.part16.integer, 0x6B, 1);
        BossRecoveryPickup_Spawn((s16)entity->posX.part16.integer, 0x20, 1, 0x60, 1);
        entity->state = 3;
        break;
    case 3:
        if (REWARD_SCALE(entity) > 0)
        {
            REWARD_SCALE(entity) -= 0x400;
            break;
        }
        ObjAffine_Free(entity->affineIndex);
    delete:
        EntityDelete(entity);
        return;
    }
    if (++REWARD_COUNTER(entity) & 1)
    {
        u16 angle = RandomNumberGenerator();
        s32 s, dx, dy;
        struct EwramData_EntityData *debris;
        s = Sin((s16)angle + 0x4000);
        dx = FixedToInt(REWARD_SCALE(entity) << 5) * s;
        s = Sin((s16)angle);
        if ((REWARD_SCALE(entity) << 5) < 0)
            dy = -(-(REWARD_SCALE(entity) << 5) >> 16) * s;
        else
            dy = ((REWARD_SCALE(entity) << 5) >> 16) * s;
        debris = Effect_SpawnShared(entity->posX.whole + dx, entity->posY.whole + dy, 0x10, 6);
        if (debris != NULL)
        {
            s32 r = RandomNumberGenerator();
            s32 one = 1;
            debris->flipX = r & one;
            debris->flipY = RandomNumberGenerator() & one;
            debris->accelX.whole = -dx / 256;
            debris->accelY.whole = -dy / 256;
        }
    }
    REWARD_ROTATION(entity) += 0x1A0;
    ObjAffine_Set(REWARD_ROTATION(entity), REWARD_SCALE(entity), REWARD_SCALE(entity),
                 entity->flipX, 0, entity->affineIndex);
}
