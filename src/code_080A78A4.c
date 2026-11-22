#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void TsuchinokoDroplet_Update(struct EwramData_EntityData *);
extern u8 sTsuchinokoGfx[], sTsuchinokoSpriteSheet[];
static inline u8 LoadTsuchinokoGraphics(struct EwramData_EntityData *parent, struct EwramData_EntityData *child)
{
    s32 tiles = ObjGfx_Find(sTsuchinokoGfx);
    if (tiles < 0)
        return 0;
    Sprite_Attach(child, sTsuchinokoSpriteSheet, tiles, parent->unk_510.part8.basePalette);
    child->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/* Create a randomly flipped falling effect with no contact callback. */
void Tsuchinoko_SpawnDroplet(struct EwramData_EntityData *parent)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)TsuchinokoDroplet_Update);
    if (child != NULL)
    {
        s32 zero;
        u8 *frame;
        if (!LoadTsuchinokoGraphics(parent, child))
            EntityDelete(child);
        child->scrollWithBg = 1;
        frame = &child->animFrame;
        zero = 0;
        *frame = 18;
        {
            u8 flags = child->hit.flags;
            flags |= 2;
            flags |= 4;
            child->hit.flags = flags;
        }
        child->spriteLayer = parent->spriteLayer - 1;
        ENTITY_CREATOR(child) = parent;
        child->flipX = RandomNumberGenerator() % 2;
        child->flipY = RandomNumberGenerator() % 2;
        child->posX.part16.integer = parent->posX.part16.integer;
        child->posY.part16.integer = parent->posY.part16.integer;
        child->velX.whole = (RandomNumberGenerator() % 0x20000) - 0x10000;
        child->velY.whole = -0x10000 - (RandomNumberGenerator() % 0x10000);
        child->accelX.whole = zero;
        child->accelY.whole = 0x1800;
        child->unk_518.part8.spawnIndex = parent->unk_518.part8.spawnIndex;
        child->unk_510.part8.basePalette = parent->unk_510.part8.basePalette;
        child->attack = parent->attack;
    }
}
