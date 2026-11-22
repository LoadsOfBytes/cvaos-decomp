/* Object13Update. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "structs/ewram.h"
void Object13Update(struct EwramData_EntityData *entity)
{
    s32 *data = (s32 *)&entity->unk_4FC;
    if (entity->state == 0xFF)
    {
        if (entity->timer > 1) { EntityDelete(entity); return; }
        entity->timer++;
    }
    if (entity->accelX.whole > 0x2300) entity->accelX.whole = 0x2300;
    if (entity->unk_514.part16.spawnParam1 != 0) {
        u32 *base = gEwramData->gameState.gameFlagsTail;
        u32 *word = ({ s32 o_ = (entity->unk_514.part16.spawnParam1 >> 5) * 4; u32 *b_ = gEwramData->gameState.gameFlagsTail; (u32 *)((u8 *)b_ + o_); });
        u32 bit = 1 << (entity->unk_514.part16.spawnParam1 & 31);
        if (!(*word & bit)) {
            register s32 amplitude asm("r5"); /* FAKEMATCH: pinned register */
            s32 threshold;
            if (entity->unk_514.part16.spawnParam0 == 0) { amplitude = entity->accelX.whole; threshold = 0x2100; }
            else { amplitude = entity->accelX.whole; threshold = 0x1200; }
            if (amplitude > threshold && (u32)(entity->velX.whole - 0xAC01) <= 0x7FE) {
                entity->velX.whole -= 0x40;
                entity->accelX.whole = ({ register s32 t asm("r0") = amplitude - 0x100; t; }); /* FAKEMATCH: pinned register */
                *word |= bit;
            } } }
    if (entity->accelX.whole > 0)
    {
        entity->accelX.whole -= 4;
        entity->velX.whole = (entity->velX.whole + 0x200) & 0xFFFF;
    }
    else
    {
        entity->accelX.whole = 0;
        entity->velX.whole = 0;
    }
    data[1] = data[2];
    data[2] = (Sin(entity->velX.whole) * entity->accelX.whole) / 0x10000;
    data[3] = entity->posX.whole;
    { u32 tmp_ = entity->posY.whole; data[4] = tmp_; }
}
