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
 * @brief Create frame-13 debris at fixed-point x/y, inheriting facing, layer and palette; lifetime 30 ticks.
 */
void Zombie_SpawnFallingDebris(struct EwramData_EntityData *parent, s32 x, s32 y)
{
    struct EwramData_EntityData *part = Entity_CreateObject((u32 *)ZombieDebris_Update);
    if (part)
    {
        s32 graphics;
        part->posX.whole = x;
        part->posY.whole = y;
        part->flipX = parent->flipX;
        part->spriteLayer = parent->spriteLayer;
        graphics = ObjGfx_Find(sZombieGfx);
        Sprite_Attach(part, sZombieSpriteSheet, graphics, parent->unk_510.part8.basePalette);
        part->animFrame = 13;
        part->timer = 30;
        part->accelY.whole = 0xA00;
        part->scrollWithBg = 1;
        part->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
