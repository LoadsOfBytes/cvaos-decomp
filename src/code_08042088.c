#include "entity_hit.h"

/**
 * @brief 42088 | Mutual collision check of entities a and b (each direction
 * via Hit_Check); fills res and calls the onHit / onHurt callbacks.
 *
 * @return res->flags (bit 0: a was hit by b, bit 1: b was hit by a)
 */
u32 Hit_CheckPair(struct HitResult *res, u8 *a, u8 *b)
{
    struct HitInfo *ia;
    struct HitInfo *ib;
    u32 r;
    u8 type;

    ia = (struct HitInfo *)(a + 0x70);
    ib = (struct HitInfo *)(b + 0x70);
    res->flags = 0;
    res->typeB = 0;
    res->typeA = 0;
    res->resB = 0;
    res->resA = 0;
    if (!(ia->flags & 4) && !(ib->flags & 2)
        && (type = ib->type, ia->hitBy[0] != type) && ia->hitBy[1] != type && ia->hitBy[2] != type
        && (ia->hitBy[0] == 0 || ia->hitBy[1] == 0 || ia->hitBy[2] == 0))
    {
        r = Hit_Check(b, a);
        if (r != 0)
        {
            res->flags |= 1;
            res->typeA = ib->type;
            res->resA = r;
        }
    }
    if (!(ia->flags & 2) && !(ib->flags & 4)
        && (type = ia->type, ib->hitBy[0] != type) && ib->hitBy[1] != type && ib->hitBy[2] != type
        && (ib->hitBy[0] == 0 || ib->hitBy[1] == 0 || ib->hitBy[2] == 0))
    {
        r = Hit_Check(a, b);
        if (r != 0)
        {
            res->flags |= 2;
            res->typeB = ia->type;
            res->resB = r;
        }
    }
    if (res->flags & 1)
    {
        if (ia->onHurt != NULL)
            ia->onHurt(a, b, res);
        if (ib->onHit != NULL)
            ib->onHit(b, a, res);
    }
    if (res->flags & 2)
    {
        if (ia->onHit != NULL)
            ia->onHit(a, b, res);
        if (ib->onHurt != NULL)
            ib->onHurt(b, a, res);
    }
    return res->flags;
}
