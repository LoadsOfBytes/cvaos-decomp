#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

struct OrbitSpeed {
    s16 x;
    s16 y;
};
extern const struct OrbitSpeed sBaloreIntroPieceOrbitSpeeds[16];
/* The original reads the y speeds through a pointer two bytes into the table. */
#define ORBIT_SPEED_Y ((const s16 *)((const u8 *)sBaloreIntroPieceOrbitSpeeds + 2))

/**
 * @brief B5F1C | Orbiting part of a boss (parent unk_4FC): spawn with the
 * parent graphics, fly out along angle unk_504 when the parent reaches state
 * 3, then circle it with a shrinking radius from state 4 and delete itself;
 * deleted at once when the parent enters state 10.
 */
void BaloreIntroPiece_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = entity->unk_4FC.unk_4FC_entity;
    s32 i;
    s32 angle;
    u8 timer;

    if (parent->state == 10)
        entity->state = 6;
    switch (entity->state)
    {
    case 0:
        Sprite_Attach(entity, (void *)0x08244D4C, parent->unk_4F8.bytes[3], parent->unk_510.part8.basePalette);
        Anim_StartIndexed(entity, 1, 2, 1);
        entity->animTimer = RandomNumberGenerator() & 7;
        entity->drawFunc = (u32 *)Sprite_DrawEntity;
        entity->spriteLayer = 9;
        entity->state = 1;
    case 1:
        if (parent->state != 3)
            break;
        i = entity->unk_504.part8.unk_504;
        if (i > 15)
            i = 31 - i;
        angle = i << 11;
        entity->velX.whole = Sin(angle + 0x4000);
        entity->accelX.whole = entity->velX.whole / 4;
        entity->velY.whole = Sin(angle) / 2;
        entity->accelY.whole = entity->velY.whole / 4;
        entity->state = 2;
        entity->timer = entity->unk_504.part8.unk_504;
        break;
    case 2:
        timer = entity->timer;
        if (timer != 0)
            goto count;
        Anim_StartIndexed(entity, 2, 2, 1);
        entity->timer = 0x50;
        entity->state = 3;
    case 3:
        EntityAccelerateAndMove(entity, 0x80000);
        timer = entity->timer;
        if (timer != 0)
        {
    count:
            entity->timer = timer - 1;
            break;
        }
        entity->state = 4;
        break;
    case 4:
        if (parent->state != 4)
            break;
        entity->unk_500.part16.unk_500 = RandomNumberGenerator();
        entity->unk_500.part16.unk_502 = entity->unk_504.part8.unk_504 + 0x80;
        entity->velX.part16.integer = sBaloreIntroPieceOrbitSpeeds[entity->unk_504.part8.unk_504 & 0xF].x;
        entity->velY.part16.integer = ORBIT_SPEED_Y[(entity->unk_504.part8.unk_504 & 0xF) * 2];
        entity->state = 5;
        break;
    case 5:
        entity->unk_500.part16.unk_500 += 0x300;
        entity->posX.whole = parent->posX.whole + entity->velX.whole
            + Sin(entity->unk_500.part16.unk_500 + 0x4000) * entity->unk_500.part16.unk_502;
        entity->posY.whole = parent->posY.whole + entity->velY.whole
            + Sin(entity->unk_500.part16.unk_500) * entity->unk_500.part16.unk_502;
        if (entity->unk_500.part16.unk_502 > 0x20)
            entity->unk_500.part16.unk_502 -= entity->unk_500.part16.unk_502 >> 5;
        if (--entity->unk_500.part16.unk_502 <= 0)
            entity->state = 6;
        break;
    case 6:
        EntityDelete(entity);
        return;
    }
    Anim_Update(entity);
}
