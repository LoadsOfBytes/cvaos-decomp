/* SkillNeedlesDraw. */
#include "code_08009A0.h"
#include "code/code_08039340.h"

struct Unk_08028018 {
    s16 radius;
    s16 angle;
};

/* Four (radius 8.8, angle) pairs at entity offset 0x20 (unk_504..unk_510);
 * addressed from unk_4F4 + 0x10 like the original. */
struct Unk_08028018_Block {
    u8 pad_0[0x10];
    struct Unk_08028018 parts[4];
};

/**
 * @brief Draw callback set by SkillNeedlesUse: draw once unless ext.skillNeedles.spread is
 * set; otherwise draw up to four copies on circles around the entity,
 * alternating halves by timer parity, with animFrame set to the index.
 */
void SkillNeedlesDraw(struct EwramData_EntityData *entity)
{
    s32 x, y;
    s32 i;
    struct Unk_08028018_Block *block;

    if ((u8)entity->ext.skillNeedles.spread == 0)
    {
        Sprite_Draw(entity);
        return;
    }
    x = entity->posX.whole;
    y = entity->posY.whole;
    block = (struct Unk_08028018_Block *)&entity->ext.raw.unk_4F4;
    for (i = 0; i <= 3; i++)
    {
        if ((entity->timer & 1) != (i & 1))
        {
            s32 r = block->parts[i].radius / 256;
            s32 a = block->parts[i].angle;
            entity->posX.whole = r * Sin(a + 0x4000) + x;
            entity->posY.whole = r * Sin(a) + y;
            entity->animFrame = i;
            Sprite_Draw(entity);
        }
    }
    entity->posX.whole = x;
    entity->posY.whole = y;
}
