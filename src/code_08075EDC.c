/* The caller selects one of five valid directions (0..4) for a spread shot. */
#include "code_08009A0.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
extern void HarpyFeather_Update(struct EwramData_EntityData *);
extern u8 HarpyFeather_LoadGraphics(struct EwramData_EntityData *,struct EwramData_EntityData *);
extern void HarpyFeather_EnableHitBoxes(struct EwramData_EntityData *);
extern void EntitySetVelocityFromAngle(struct EwramData_EntityData *,s32,s32);
void Harpy_ThrowFeather(struct EwramData_EntityData *owner,s32 kind)
{
    struct EwramData_EntityData *projectile=EntityCreateInRange(0x1B,0x48,(u32 *)HarpyFeather_Update);
    if(projectile)
    {
        u32 angle;
        u32 flipped;
        if(!HarpyFeather_LoadGraphics(owner,projectile))
            EntityDelete(projectile);
        projectile->scrollWithBg=1;
        HarpyFeather_EnableHitBoxes(projectile);
        projectile->spriteLayer=owner->spriteLayer+1;
        ENTITY_CREATOR(projectile)=owner;
        projectile->flipX=owner->flipX;
        if(owner->flipX)
            projectile->posX.part16.integer=owner->posX.part16.integer+1;
        else
            projectile->posX.part16.integer=owner->posX.part16.integer-1;
        projectile->posY.part16.integer=owner->posY.part16.integer+2;
        switch(kind)
        {
            case 0:
                angle=0x4000;
                projectile->animFrame=12;
                break;
            case 1:
                flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(projectile)) & 0x40;
                angle=0x5000;
                if(flipped) angle=0x3000;
                projectile->animFrame=11;
                break;
            case 2:
                flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(projectile)) & 0x40;
                angle=0x6000;
                if(flipped) angle=0x2000;
                projectile->animFrame=10;
                break;
            case 3:
                flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(projectile)) & 0x40;
                angle=0x7000;
                if(flipped) angle=0x1000;
                projectile->animFrame=9;
                break;
            case 4:
                flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(projectile)) & 0x40;
                angle=0x8000;
                if(flipped) angle=0;
                projectile->animFrame=8;
                break;
        }
        projectile->ext.harpyFeather.angle=angle;
        EntitySetVelocityFromAngle(projectile,angle,0x30000);
        projectile->spawnIndex=owner->spawnIndex;
        projectile->basePalette=owner->basePalette;
        projectile->attack=owner->attack;
    }
}
