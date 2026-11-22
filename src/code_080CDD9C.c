/* Rotate Alastor's collision rectangle with its angle and facing. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void HitBox_SetAttackRaw(struct EwramData_EntityData *,s32);
extern s32 HitBox_SetBodyRaw(struct EwramData_EntityData *,s32);
/**
 * @brief Compute the offset and orientation-dependent rectangle from
 * angle/facing and write it to both body and attack hit boxes.
 */
void Alastor_UpdateHitBoxes(struct EwramData_EntityData *entity)
{
    s32 angle=entity->unk_500.part16.unk_500;
    s32 horizontal=(angle+0x4000)&0xFFFF;
    s32 vertical=(angle+0xC000)&0xFFFF;
    s32 side=vertical;
    s8 width=48;
    s8 height=8;
    s8 x;
    u8 y;
    u32 box;
    if (!entity->flipX)
        horizontal+=0x8000;
    if (vertical>0xA000)
        side=vertical-0x8000;
    if ((u32)(side-0x2001)<=0x3FFE)
    {
        width=8;
        height=48;
    }
    x=FixedMultiply(Sin(horizontal+0x4000),0x2A0000)>>16;
    y=FixedMultiply(Sin(vertical),0x2A0000)>>16;
    if ((s8)x>0)
        x-=width;
    box=((u8)x | (y<<8)) + ((u8)width<<16) + ((u8)height<<24);
    HitBox_SetAttackRaw(entity,box);
    HitBox_SetBodyRaw(entity,box);
}
