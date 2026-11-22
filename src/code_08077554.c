/* Spawn a projectile along the head's current direction. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void WhiteDragonBreath_Update(struct EwramData_EntityData *);
extern u8 WhiteDragonBreath_LoadGraphics(struct EwramData_EntityData *,struct EwramData_EntityData *);
extern void WhiteDragonBreath_EnableHitBoxes(struct EwramData_EntityData *);
/**
 * @brief Spawn a breath fireball from the head in its aim direction (sound 0x77
 * at the caller).
 */
void WhiteDragon_SpawnBreath(struct EwramData_EntityData *owner)
{
    struct EwramData_EntityData *projectile=EntityCreateInRange(0x1B,0x48,(u32 *)WhiteDragonBreath_Update);
    if(projectile)
    {
        s16 *direction;
        s32 horizontal,vertical,speed;
        if(!WhiteDragonBreath_LoadGraphics(owner,projectile)) EntityDelete(projectile);
        projectile->scrollWithBg=1;
        WhiteDragonBreath_EnableHitBoxes(projectile);
        projectile->spriteLayer=owner->spriteLayer-1;
        projectile->unk_4F4.unk_4F4_entity=owner;
        projectile->flipX=owner->flipX;
        if(owner->flipX)
            projectile->posX.part16.integer=owner->posX.part16.integer;
        else
            projectile->posX.part16.integer=owner->posX.part16.integer;
        projectile->posY.part16.integer=owner->posY.part16.integer;
        direction=&owner->accelX.part16.integer;
        horizontal=Sin(*direction+0x4000);
        vertical=Sin(*direction);
        speed=0x40000;
        projectile->velX.whole=FixedMultiply(horizontal,speed);
        projectile->velY.whole=FixedMultiply(vertical,speed);
        projectile->unk_518.part8.spawnIndex=owner->unk_518.part8.spawnIndex;
        projectile->unk_510.part8.basePalette=owner->unk_510.part8.basePalette;
        projectile->attack=owner->attack;
    }
}
