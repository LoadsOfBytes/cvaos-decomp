/* Zombie debris and temporary effects. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void ZombiePiece_Update(struct EwramData_EntityData *);
extern u8 sZombieGfx[], sZombieSpriteSheet[];
extern void ZombieDebris_Update(struct EwramData_EntityData *);
extern void ZombieTemporarySprite_Update(struct EwramData_EntityData *);
/**
 * @brief Create a frame-0 sprite at fixed-point x/y, one layer above the parent, with a 120-tick counter.
 */
void Zombie_SpawnTemporarySprite(struct EwramData_EntityData *parent, s32 x, s32 y)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, (u32 *)ZombieTemporarySprite_Update);
    if (part)
    {
        s32 zero;
        s32 graphics;
        part->posX.whole = x;
        part->posY.whole = y;
        part->flipX = parent->flipX;
        part->spriteLayer = parent->spriteLayer + 1;
        zero = 0;
        graphics = ObjGfx_Find(sZombieGfx);
        Sprite_Attach(part, sZombieSpriteSheet, graphics, parent->basePalette);
        part->animFrame = zero;
        part->ext.zombieSprite.lifetime = 120;
        part->scrollWithBg = 1;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
