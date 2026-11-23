/* Draw the entity twice with mirrored affine matrices (rotation from unk_4F8), restoring position, frame and flip. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/entity_render_flags.h"

struct AffineSlot16 { u16 slot:5; u16 rest:3; };
/**
 * @brief Draw the gate entity twice with mirrored affine matrices from its
 * opening angle unk_4F8 (0x4000 = fully open draws it flipped without
 * affine), restoring position, frame and flip.
 */
void Gate_Draw(struct EwramData_EntityData *e)
{
    s32 savedX;
    u8 *p549;
    u8 saved549;
    u32 savedFlip;
    s32 d;
    s32 m;

    savedX = e->posX.whole;
    p549 = &e->animFrame;
    saved549 = *p549;
    savedFlip = e->flipX;
    if (e->ext.gate.openAngle == 0x4000)
    {
        e->affine = 0;
        e->flipX ^= 1;
    }
    else
    {
        if (e->ext.gate.openAngle != 0)
        {
            e->affine = 1;
            ObjAffine_Set(0, Sin(e->ext.gate.openAngle), 0x10000, e->flipX ^ 1, 0, e->ext.gateDraw.affine[0]);
            m = 0x1F;
            
            e->affineIndex = ({ u32 b_ = e->ext.gateDraw.affine[0]; asm("" ::: "r0"); /* FAKEMATCH: forget r0 == mask so reload copies it from r8 */ b_; }) & m;
            Sprite_Draw(e);
            ObjAffine_Set(0, Sin(e->ext.gate.openAngle + 0x4000), 0x10000, e->flipX, 0, e->ext.gateDraw.affine[1]);
            e->affineIndex = ({ u32 b_ = e->ext.gateDraw.affine[1]; b_; }) & m;
            
            asm("" :: "r"(m)); /* FAKEMATCH: lengthen mask life so zero wins r5 in local-alloc */
            d = (Sin(e->ext.gate.openAngle) << 5) - 0x10000;
            {
                s32 v;
                s32 x = e->posX.whole;
                if (e->flipX)
                    v = x - d;
                else
                    v = x + d;
                e->posX.whole = v;
                asm("" :: "r"(m)); /* FAKEMATCH: lengthen mask life so zero wins r5 in local-alloc */
            }
        }
        else
        {
            { u8 *q_ = ENTITY_RENDER_FLAGS_HIGH_ADDRESS(e); u8 f_ = *q_; register u32 m_ asm("r0") = ~0x10; m_ &= f_; *q_ = m_; /* FAKEMATCH: mask/result in r0 */ }
            *p549 = saved549 + 2;
            Sprite_DrawEntity(e);
        }
        *p549 = saved549 + 1;
    }
    Sprite_Draw(e);
    e->posX.whole = savedX;
    e->animFrame = saved549;
    e->flipX = savedFlip;
}
