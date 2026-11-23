#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern s32 Enemy_IsStunnedNoFlash(struct EwramData_EntityData *);
extern void PlaySong(u16 song);
#define KIND(e) ((u8)e->ext.manticoreTail.kind)
#define TIMER(e) (e->ext.manticoreTail.timer)
#define STEP(e) (e->ext.manticoreTail.step)
#define VALUE(e) (e->ext.manticoreTail.link.value)
#define SETANGLE(e) \
    do { \
        if ((e)->flipX) angle = -0x2000 - VALUE(e); \
        else angle = VALUE(e) + 0x12000; \
        (e)->velY.whole = angle; \
    } while (0)

/* Swing a linked part (chain link of ManticoreTailSegment_Update) around its owner depending on the owner's state. */
void ManticoreTail_UpdateSwing(struct EwramData_EntityData *e)
{
    struct EwramData_EntityData *owner = ENTITY_OWNER(e);
    struct EwramData_EntityData *part = e->ext.manticoreTail.anchor;
    s32 angle, x, y, r;

    if (Enemy_IsStunnedNoFlash(owner) == 0)
    {
        if (owner->animScriptIndex == 0)
        {
            if (KIND(e) == 6)
            {
                switch (owner->animStep)
                {
                case 0:
                case 5:
                case 6:
                case 7:
                    if (VALUE(e) < 0x4000) VALUE(e) += 0x60;
                    break;
                case 1:
                case 2:
                case 3:
                case 4:
                    if (VALUE(e) > 0x3800) VALUE(e) -= 0x60;
                    break;
                }
                SETANGLE(e);
            }
        }
        else if (owner->animScriptIndex == 1)
        {
            if (KIND(e) == 6)
            {
                switch (owner->animStep)
                {
                case 2:
                case 3:
                case 8:
                case 9:
                    if (VALUE(e) > 0x3800) VALUE(e) -= 0x60;
                    break;
                default:
                    if (VALUE(e) < 0x4000) VALUE(e) += 0x40;
                    break;
                }
                SETANGLE(e);
            }
        }
        else if ((u8)(owner->animScriptIndex - 2) <= 1)
        {
            if (KIND(e) == 6)
            {
                u8 step = STEP(e);
                s32 t;
                if (STEP(e) == 0)
                {
                    STEP(e) = step + 1;
                    TIMER(e) = 0;
                }
                else if (STEP(e) == 1)
                {
                    t = TIMER(e) + 1;
                    TIMER(e) = t;
                    if (t & 8)
                    {
                        s32 d = VALUE(e) - 0x8000;
                        s32 ad = d;
                        if (d < 0) ad = -d;
                        if (ad > 0x40)
                        {
                            if (d > 0) VALUE(e) -= 0x40;
                            else VALUE(e) += 0x40;
                        }
                        if (e->flipX) { t = TIMER(e); angle = ({ s32 a_ = -0x2000 - ((~t & 7) << 9); a_; }) - VALUE(e); }
                        else { t = TIMER(e); angle = VALUE(e) + ({ s32 a_ = ((~t & 7) << 9) + 0x12000; a_; }); }
                    }
                    else
                    {
                        if (e->flipX) { angle = ({ s32 a_ = -0x2000 - ((t & 7) << 9); a_; }) - VALUE(e); }
                        else { angle = VALUE(e) + ({ s32 a_ = ((t & 7) << 9) + 0x12000; a_; }); }
                    }
                    e->velY.whole = angle;
                    if ((u8)t > 0x1F)
                    {
                        STEP(e)++;
                        TIMER(e) = 0;
                    }
                }
                else if (STEP(e) == 2)
                {
                    TIMER(e)++;
                    if (TIMER(e) > 0x13)
                    {
                        TIMER(e) = 0;
                        STEP(e) = step + 1;
                        PlaySong(0x86);
                    }
                }
                else if (STEP(e) == 3)
                {
                    VALUE(e) += 0x800;
                    if (VALUE(e) > 0x7FFF)
                    {
                        STEP(e) = step + 1;
                        VALUE(e) = 0x8000;
                    }
                }
                else if (STEP(e) == 4)
                {
                    TIMER(e)++;
                    if (TIMER(e) > 0xB)
                    {
                        TIMER(e) = 0;
                        STEP(e) = step + 1;
                    }
                }
                else if (STEP(e) == 5)
                {
                    VALUE(e) -= 0x400;
                    if (VALUE(e) < 0x4000)
                    {
                        VALUE(e) = 0x4000;
                        STEP(e) = step + 1;
                        TIMER(e) = 0;
                    }
                }
                if (STEP(e) != 1) SETANGLE(e);
            }
        }
        else if (owner->animScriptIndex == 4)
        {
            if (KIND(e) == 6) SETANGLE(e);
        }
        else if (owner->animScriptIndex == 7)
        {
            if (KIND(e) == 6) SETANGLE(e);
        }
        else if (owner->animScriptIndex == 8)
        {
            u8 kind = e->ext.raw.unk_500.whole; /* word read of ext.manticoreTail.kind (the byte member changes the code) */
            if (kind == 6)
            {
                if (owner->timer >= 10 && owner->timer <= 14)
                {
                    e->posX.whole = owner->posX.whole;
                    e->posY.whole = owner->posY.whole;
                    if (owner->flipX)
                    {
                        e->posX.whole += 0x80000;
                        e->velX.whole = (KIND(e) << 19) + 0xC0000;
                    }
                    else
                    {
                        e->posX.whole -= 0x80000;
                        e->velX.whole = -((KIND(e) << 19) + 0xC0000);
                    }
                    e->posY.whole -= 0xE0000;
                    e->flipX = owner->flipX;
                }
                else if (owner->timer > 14)
                {
                    e->posX.whole = owner->posX.whole;
                    e->posY.whole = owner->posY.whole;
                    if (owner->flipX)
                    {
                        e->posX.whole += 0xC0000;
                        e->velX.whole = (KIND(e) << 19) + 0xC0000;
                    }
                    else
                    {
                        e->posX.whole -= 0xC0000;
                        e->velX.whole = -((KIND(e) << 19) + 0xC0000);
                    }
                    e->posY.whole -= 0xE0000;
                    e->flipX = owner->flipX;
                }
                SETANGLE(e);
            }
            else
            {
                if (owner->timer >= 10 && owner->timer <= 14)
                {
                    e->posX.whole = owner->posX.whole;
                    e->posY.whole = owner->posY.whole;
                    if (owner->flipX)
                    {
                        if (kind == 0) e->velX.whole = (KIND(e) << 19) + 0x60000;
                        else e->velX.whole = (KIND(e) << 19) + 0xA0000;
                        e->posX.whole += 0x80000;
                    }
                    else
                    {
                        if (kind == 0) e->velX.whole = -((KIND(e) << 19) + 0x60000);
                        else e->velX.whole = -((KIND(e) << 19) + 0xA0000);
                        e->posX.whole -= 0x80000;
                    }
                }
                else if (owner->timer > 14)
                {
                    e->posX.whole = owner->posX.whole;
                    e->posY.whole = owner->posY.whole;
                    if (owner->flipX)
                    {
                        if (kind == 0) e->velX.whole = (KIND(e) << 19) + 0x60000;
                        else e->velX.whole = (KIND(e) << 19) + 0xA0000;
                        e->posX.whole += 0xC0000;
                    }
                    else
                    {
                        if (kind == 0) e->velX.whole = -((KIND(e) << 19) + 0x60000);
                        else e->velX.whole = -((KIND(e) << 19) + 0xA0000);
                        e->posX.whole -= 0xC0000;
                    }
                }
                else
                    goto tail;
                e->posY.whole -= 0x150000;
                e->flipX = owner->flipX == 0;
            }
        }
    }
    else
    {
        if (KIND(e) == 6) SETANGLE(e);
    }
tail:
    if (KIND(e) == 6)
    {
        x = (VALUE(e) - 0x7000) / 4;
        x = FixedMultiply(e->velX.whole, x + 0x4000);
        x = FixedDivide(x, 0x4000);
        e->velX.whole = x;
        if (e->flipX) angle = 0x10000 - VALUE(e);
        else angle = VALUE(e);
        x = Sin(angle + 0x4000);
        y = Sin(angle);
        x = FixedMultiply(x, e->velX.whole);
        y = FixedMultiply(y, e->velX.whole);
        e->posX.whole += x;
        e->posY.whole += y;
    }
    else
    {
        /* do-while(0) wrappers raise register priority of r (allocation as in ROM) */
        do { r = VALUE(part); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        x = (r - 0x4000) * 5 / 6 * 24 + 0x70000;
        y = (KIND(e) << 16) + x - 0x60000;
        r = FixedMultiply(r, y);
        do { r = FixedDivide(r, x); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        x = (VALUE(part) - 0x7000) / 4;
        x = FixedMultiply(e->velX.whole, x + 0x4000);
        x = FixedDivide(x, 0x4000);
        e->velX.whole = x;
        do { if (!e->flipX) r = 0x10000 - r; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        e->velY.whole = angle;
        x = Sin(r + 0x4000);
        do { y = Sin(r); } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
        x = FixedMultiply(x, e->velX.whole);
        y = FixedMultiply(y, e->velX.whole);
        e->posX.whole += x;
        e->posY.whole += y;
        e->velX.whole = 0;
        e->velY.whole = 0;
    }
}
