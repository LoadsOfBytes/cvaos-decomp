/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   ZombieSoldier_SpawnPiece = ZombieOfficer_SpawnPiece */
#include "code_08009A0.h"
#include "zombie_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern void ZombieSoldierPiece_Update(struct EwramData_EntityData *entity);
extern u8 sZombieSoldierGfx[];
extern u8 sZombieSoldierSpriteSheet[];

/**
 * @brief 92FCC | Same as ZombieOfficer_SpawnPiece.
 */
void ZombieSoldier_SpawnPiece(struct EwramData_EntityData *entity, s32 x, s32 y, s32 vx, s32 vy, s32 frame)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)ZombieSoldierPiece_Update);

    if (child != NULL)
    {
        s32 gfx;

        child->posX.whole = entity->posX.whole + x;
        child->posY.whole = entity->posY.whole + y;
        ZOMBIE_PIECE_FLOOR_Y(child) = entity->posY.whole;
        child->flipX = entity->flipX;
        child->spriteLayer = entity->spriteLayer - 1;
        gfx = ObjGfx_Find(sZombieSoldierGfx);
        Sprite_Attach(child, sZombieSoldierSpriteSheet, gfx, entity->unk_510.part8.basePalette);
        child->animFrame = frame;
        child->accelY.whole = 0x1800;
        child->velX.whole = vx;
        child->velY.whole = vy;
        child->scrollWithBg = 1;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}
