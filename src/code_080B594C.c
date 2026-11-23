/* Boss part effect: sparks, debris bursts and a fading palette countdown. */
#include "code_08009A0.h"
#include "balore_entity.h"
#include "code_08011DD0.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
extern void ObjPalette_StartColorFade(u16 r, u16 g, u16 b, s32 from, s32 to, s32 frames);
extern void BgPalette_StartColorFade(u16 r, u16 g, u16 b, s32 from, s32 to, s32 frames);
extern void ObjPalette_LoadRange(s32 palette, u16 first, u16 count);
extern struct EwramData_EntityData *Effect_SpawnShared(s32, s32, s32, s32);
extern void SoulPickup_SpawnFromEnemyDefinition(s32, s32, s32, s32);
extern void BossRecoveryPickup_Spawn(s32, s32, s32, s32, s32);

static inline u8 *Slot(u8 *base, u32 i) { return base + i; }

/**
 * @brief Death state (state 4): defeat sequence; returns nonzero when the
 * entity can be removed.
 */
s32 Balore_UpdateDeath(struct EwramData_EntityData *entity)
{
    s32 one;
    struct EwramData_EntityData *fx;
    s32 a;
    u8 b;
    u8 idx;
    u8 *p;
    s32 x, y;

    switch (entity->subState)
    {
    case 0:
        if (!(gEwramData->gameState.stateFlags & 0x10))
        {
            BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x10000, 0, 4);
            ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x10000, 0, 4);
        }
        entity->subState = 1;
        entity->timer = 4;
    case 1:
        if (entity->timer)
            goto dec;
        if (!(gEwramData->gameState.stateFlags & 0x10))
        {
            BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x20);
            ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x20);
        }
        entity->timer = 0x20;
        entity->subState = 3;
        PlaySong(0x1D6);
        break;
    case 2:
        if (entity->timer)
            goto dec;
        entity->timer = 0x80;
        entity->subState = 3;
        
        break;
    case 3:
        one = 0;
        if (entity->timer & 1)
        {
             x = ((s16)entity->posX.part16.integer - 0x10) << 16; 
            y = ((s16)entity->posY.part16.integer - 0x10) << 16;
             fx = Effect_SpawnShared(x, y, 0xD, 8);
            if (fx != NULL)
            {
                s32 r = RandomNumberGenerator();
                one = 1; fx->flipX = r & one;
                fx->flipY = RandomNumberGenerator() & one; asm("" :: "r"(one)); /* FAKEMATCH: one stays live, so ands binds to the RNG result */
            }
        }
        if (entity->timer)
            goto dec;
        entity->timer = 0x80;
        entity->subState = 4;
        break;
    case 4:
        if (entity->timer > 0x20)
        {
             x = ((s16)entity->posX.part16.integer - 0x15) << 16; 
            y = ((s16)entity->posY.part16.integer - 9) << 16;
            fx = Effect_SpawnShared(x, y, 0xF, 7);
            if (fx != NULL)
            {
                a = (RandomNumberGenerator2() & 0xFF) << 8;
                fx->velX.whole = Sin(a + 0x4000) * entity->timer / 32;
                fx->velY.whole = entity->timer * Sin(a) / 32;
                fx->accelX.whole = fx->velX.whole / 32;
                fx->accelY.whole = fx->velY.whole / 32;
                {
                s32 r = RandomNumberGenerator();
                one = 1; fx->flipX = r & one;
                fx->flipY = RandomNumberGenerator() & one; asm("" :: "r"(one)); /* FAKEMATCH: one stays live, so ands binds to the RNG result */
                }
                if (!(entity->timer & 7))
                    PlaySong(0x77);
            }
        }
        if (BALORE_BODY_SCALE(entity) > 0x8000)
            BALORE_BODY_SCALE(entity) -= 0x100;
        if (entity->timer)
        {
            entity->timer--;
            {
                register u8 s asm("r3") = entity->unk_510.part8.basePalette; /* FAKEMATCH: pinned register */
                b = 7 - (*(vu8 *)&entity->timer >> 4);
                idx = s - 10;
            p = Slot(gEwramData->gameState.sharedObjPaletteVariants, idx); asm("" :: "r"(s)); /* FAKEMATCH: s keeps r3 busy, so reloads use r4 */
            if (*p != b)
            {
                *p = b;
                { s32 a_ = gEwramData->gameState.sharedObjPaletteSources[idx]; asm("" :: "r"(s)); ObjPalette_LoadRange(a_, b, 1); } /* FAKEMATCH: same */
            }
            break;
            }
        }
        BALORE_FLAGS(entity) |= 4;
        entity->subState = 5;
        entity->timer = 0x20;
        break;
    case 5:
        if (entity->timer)
            goto dec;
        PlaySong(0x7C);
        ScreenShake_Request(0, 0x20, 0, 7);
        entity->subState = 6;
        entity->timer = 0x20;
        break;
    case 6:
        if (entity->timer)
            goto dec;
        SoulPickup_SpawnFromEnemyDefinition((s16)entity->posX.part16.integer - 0x10, (s16)entity->posY.part16.integer, entity->unk_518.part8.enemyId, 6);
        BossRecoveryPickup_Spawn((s16)entity->posX.part16.integer, 0x20, 6, 0x60, 1);
        return 1;
    dec:
        entity->timer--;
        break;
    }
    return 0;
}
