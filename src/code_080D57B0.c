/* Burst of particles in timed waves from a 4x15 offset table (BossDefeat_EmitParticleWavesUnused).
 * The timer is read through volatile like the original (fresh ldrb). */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void *memcpy(void *, const void *, unsigned long);
struct BurstOffset { s16 x, y; };
extern const struct BurstOffset sBossDefeatBurstOffsets[4][15];
extern void BossDefeatParticle_Update(struct EwramData_EntityData *);
extern void BossDefeat_RestoreEffectTiles(void);

/**
 * @brief Unreferenced defeat effect next to GrahamBreakupFragment_Update: emit
 * BossDefeatParticle_Update particles in timed waves from the 4x15
 * offset table sBossDefeatBurstOffsets.
 */
void BossDefeat_EmitParticleWavesUnused(struct EwramData_EntityData *entity)
{
    struct BurstOffset offsets[4][15];
    memcpy(offsets, sBossDefeatBurstOffsets, sizeof(offsets));
    switch (entity->state)
    {
    case 0:
        entity->timer = 8;
        entity->accelX.whole = 0x20000;
        entity->state++;
    case 1:
        if (entity->timer == 0)
        {
            entity->timer = 0x80;
            entity->state++;
            break;
        }
        entity->timer--;
        break;
    case 2:
        if (!(entity->timer % 32))
        {
            BgPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0, 0x10000, 0x20);
            ObjPalette_StartColorFade(0x1F, 0x1F, 0x1F, 0x4000, 0x10000, 0x10);
        }
        {
            u8 timer = *(vu8 *)&entity->timer;
            if (timer == 0)
            {
                entity->timer = 0x40;
                entity->state++;
                break;
            }
            if ((u8)((u32)*(vu8 *)&entity->timer % 3) == 0 && entity->stage <= 14)
            {
                s32 row, side;
                for (row = 0; row <= 3; row++)
                {
                    if (offsets[row][entity->stage].x != 0xFF)
                    {
                        for (side = 0; side <= 1; side++)
                        {
                            struct EwramData_EntityData *effect;
                            if (row == 0 && side == 0) continue;
                            effect = EntityCreateInRange(0x49, 0xCF, (u32 *)BossDefeatParticle_Update);
                            if (effect != NULL)
                            {
                                effect->scrollWithBg = 1;
                                if (side == 0)
                                    effect->posX.whole = entity->posX.whole + (offsets[row][entity->stage].x << 19);
                                else
                                    effect->posX.whole = entity->posX.whole - (offsets[row][entity->stage].x << 19);
                                effect->posY.whole = entity->posY.whole + (offsets[row][entity->stage].y << 19);
                                effect->velY.whole = 0x4000;
                                effect->accelY.whole -= 0x2000;
                                effect->spriteLayer = 0x11;
                            }
                        }
                    }
                }
                entity->stage++;
                timer = entity->timer;
            }
            entity->accelX.whole -= 0x600;
            entity->timer = timer - 1;
        }
        break;
    case 3:
        if (entity->timer == 0)
        {
            s32 col, row, side;
            for (col = 0; col <= 14; col++)
            {
                for (row = 0; row <= 3; row++)
                {
                    struct BurstOffset *offset;
                    if (((struct BurstOffset *)((u8 *)offsets + (row * 15 * 4 + col * 4)))->x == 0xFF) continue;
                    side = 0;
                    offset = (struct BurstOffset *)((u8 *)offsets + (row * 15 * 4 + col * 4));
                    for (; side <= 1; side++)
                    {
                        struct EwramData_EntityData *effect;
                        if (row == 0 && side == 0) continue;
                        effect = EntityCreateInRange(0x49, 0xCF, (u32 *)BossDefeatParticle_Update);
                        if (effect != NULL)
                        {
                            effect->scrollWithBg = 1;
                            if (side == 0) effect->posX.whole = entity->posX.whole + (offset->x << 19);
                            else effect->posX.whole = entity->posX.whole - (offset->x << 19);
                            effect->posY.whole = entity->posY.whole + (offset->y << 19);
                            effect->velY.whole = 0x4000;
                            effect->accelY.whole -= 0x2000;
                            effect->spriteLayer = 0x11;
                        }
                    }
                }
            }
            entity->timer = 0x40;
            entity->state++;
        }
        else entity->timer--;
        break;
    case 4:
        if (entity->timer == 0)
        {
            BossDefeat_RestoreEffectTiles();
            EntityDelete(entity);
            return;
        }
        entity->timer--;
        break;
    }
    if (!entity->roomActive) BossDefeat_RestoreEffectTiles();
}
