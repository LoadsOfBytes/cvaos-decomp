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
 * @brief Spawn a flesh piece (frame 13) at (x, y) with a random upward velocity
 * that falls until it passes the parent's feet (ZombiePiece_Update).
 */
void Zombie_SpawnPiece(struct EwramData_EntityData *parent, s32 x, s32 y)
{
    struct EwramData_EntityData *part = EntityCreateInRange(0x49, 0xCF, (u32 *)ZombiePiece_Update);
    if (part)
    {
        s32 graphics;
        part->posX.whole = x;
        part->posY.whole = y;
        part->ext.zombiePiece.floorY = parent->posY.whole;
        part->flipX = parent->flipX;
        part->spriteLayer = parent->spriteLayer;
        graphics = ObjGfx_Find(sZombieGfx);
        Sprite_Attach(part, sZombieSpriteSheet, graphics, parent->basePalette);
        part->animFrame = 13;
        part->accelY.whole = 0x1800;
        part->velX.whole = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
        part->velY.whole = -0x10000 - (RandomNumberGenerator() & 0xFFFF);
        part->scrollWithBg = 1;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
