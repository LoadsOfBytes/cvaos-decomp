/* The caller selects one of three valid directions (0..2) for a spread shot. */
#include "code_08009A0.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
extern void HarpyWavyFeather_Update(struct EwramData_EntityData *);
extern u8 HarpyFeather_LoadGraphics(struct EwramData_EntityData *,struct EwramData_EntityData *);
extern void HarpyFeather_EnableHitBoxes(struct EwramData_EntityData *);
extern void EntitySetVelocityFromAngle(struct EwramData_EntityData *,s32,s32);
void Harpy_ThrowWavyFeather(struct EwramData_EntityData *owner,s32 kind)
{
    struct EwramData_EntityData *projectile=EntityCreateInRange(0x1B,0x48,(u32 *)HarpyWavyFeather_Update);
    if(projectile)
    {
        u32 angle;
        u32 flipped;
        if(!HarpyFeather_LoadGraphics(owner,projectile))
            EntityDelete(projectile);
        projectile->scrollWithBg=1;
        HarpyFeather_EnableHitBoxes(projectile);
        projectile->spriteLayer=owner->spriteLayer+1;
        projectile->unk_4F4.unk_4F4_entity=owner;
        projectile->flipX=owner->flipX;
        if(owner->flipX)
            projectile->posX.part16.integer=owner->posX.part16.integer+1;
        else
            projectile->posX.part16.integer=owner->posX.part16.integer-1;
        projectile->posY.part16.integer=owner->posY.part16.integer+2;
        switch(kind)
        {
            case 0:
                flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(projectile)) & 0x40;
                angle=0x438E;
                if(flipped) angle=0x3C72;
                projectile->animFrame=14;
                break;
            case 1:
                flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(projectile)) & 0x40;
                angle=0x6000;
                if(flipped) angle=0x2000;
                projectile->animFrame=13;
                break;
            case 2:
                flipped=*(u8 *)(ENTITY_RENDER_FLAGS_ADDRESS(projectile)) & 0x40;
                angle=0x7C72;
                if(flipped) angle=0x38E;
                projectile->animFrame=14;
                break;
        }
        projectile->unk_4F8.whole=angle;
        if(projectile->flipX) projectile->timer=24;
        else projectile->timer=8;
        projectile->flipX=0;
        projectile->unk_518.part8.spawnIndex=owner->unk_518.part8.spawnIndex;
        projectile->unk_510.part8.basePalette=owner->unk_510.part8.basePalette;
        projectile->attack=owner->attack+(owner->attack>>4);
    }
}
