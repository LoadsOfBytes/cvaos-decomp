#include "code_08033CAC.h"
#include "entity_hit.h"

extern void HitBox_SetHitRect2(u8 *e, void *hb);

#define PLAYER ((u8 *)gEwramData->playerState.player)
#define INFO(e) HIT_INFO(e)

/**
 * @brief 421AC | Collision checks of entity b against the player (both
 * directions), the entity in weaponEntities and the active red / blue soul
 * entities; fills res and calls the onHit / onHurt callbacks.
 *
 * @return res->flags
 */
u32 Hit_CheckWithPlayer(struct HitResult *res, u8 *b)
{
    struct HitInfo *pi;
    struct HitInfo *ib;
    struct HitInfo *ie;
    u8 *e;
    struct EntityBoxes *hb;
    struct EntityBoxes *hb0;
    u32 r;
    u8 type0;
    u8 type1;
    u8 type2;
    u8 type3;
    u8 type4;
    s32 i;
    struct EntityBoxes *hb2;
    u8 *e2;
    struct HitInfo *ie2;

    pi = INFO(PLAYER);
    ib = INFO(b);
    res->flags = 0;
    res->typeB = 0;
    res->typeA = 0;
    res->resA = 0;
    if (!(pi->flags & 4) && !(ib->flags & 2)
        && (type0 = ib->type, pi->hitBy[0] != type0) && pi->hitBy[1] != type0 && pi->hitBy[2] != type0
        && (pi->hitBy[0] == 0 || pi->hitBy[1] == 0 || pi->hitBy[2] == 0))
    {
        r = Hit_CheckPlayerHurt(b);
        if (r != 0)
        {
            res->flags |= 1;
            res->typeA = ib->type;
            res->resA = r;
        }
    }
    if (!(pi->flags & 2) && !(ib->flags & 4)
        && (type1 = pi->type) > 1 && ib->hitBy[0] != type1 && ib->hitBy[1] != type1 && ib->hitBy[2] != type1
        && (ib->hitBy[0] == 0 || ib->hitBy[1] == 0 || ib->hitBy[2] == 0))
    {
        if (Hit_CheckPlayerAttack(b))
        {
            res->flags |= 4;
            res->typeB = pi->type;
            res->box = *(struct Box8 *)0x03001A48;
            if (ib->onHurt != NULL)
                ib->onHurt(b, PLAYER, res);
            if (pi->onHit != NULL)
                pi->onHit(PLAYER, b, res);
        }
    }
    if (!(res->flags & 2))
    {
        if (!(ib->flags & 4) && gEwramData->playerState.weaponEntities[0] != NULL)
        {
            e = (u8 *)gEwramData->playerState.weaponEntities[0];
            ie = INFO(e);
            hb0 = &gEntityBoxes[e[0x73]];
            if (!(ie->flags & 2)
                && (type2 = ie->type, ib->hitBy[0] != type2) && ib->hitBy[1] != type2 && ib->hitBy[2] != type2
                && (ib->hitBy[0] == 0 || ib->hitBy[1] == 0 || ib->hitBy[2] == 0)
                && Hit_Check(e, b))
            {
                res->flags |= 2;
                res->typeB = ie->type;
                HitBox_SetHitRect2(e, hb0);
                res->box = *(struct Box8 *)&gEwramData->hitRectLeft;
                if (ie->onHit != NULL)
                    ie->onHit(e, b, res);
                if (ib->onHurt != NULL)
                    ib->onHurt(b, e, res);
            }
        }
        if (!(res->flags & 2) && !(ib->flags & 4))
        {
            for (i = 0; i < 8; i++)
            {
                e = (u8 *)gEwramData->playerState.activeRedSoulEntities[i];
                if (e == NULL)
                    continue;
                ie = INFO(e);
                hb = &gEntityBoxes[e[0x73]];
                if (ie->flags & 2)
                    continue;
                type3 = ie->type;
                if (ib->hitBy[0] == type3 || ib->hitBy[1] == type3 || ib->hitBy[2] == type3)
                    continue;
                if (ib->hitBy[0] != 0 && ib->hitBy[1] != 0 && ib->hitBy[2] != 0)
                    continue;
                if (!Hit_Check(e, b))
                    continue;
                res->flags |= 8;
                res->typeB = ie->type;
                HitBox_SetHitRect2(e, hb);
                res->box = *(struct Box8 *)&gEwramData->hitRectLeft;
                if (ie->onHit != NULL)
                    ie->onHit(e, b, res);
                if (ib->onHurt != NULL)
                    ib->onHurt(b, e, res);
                break;
            }
        }
    }
    if (!(res->flags & 0xA) && !(ib->flags & 4))
    {
        for (i = 0; i < 8; i++)
        {
            e2 = (u8 *)gEwramData->playerState.activeBlueSoulEntities[i];
            if (e2 == NULL)
                continue;
            ie2 = INFO(e2);
            hb2 = &gEntityBoxes[e2[0x73]];
            if (ie2->type == 7 && ib->type != 10)
                continue;
            if (ie2->flags & 2)
                continue;
            type4 = ie2->type;
            if (ib->hitBy[0] == type4 || ib->hitBy[1] == type4 || ib->hitBy[2] == type4)
                continue;
            if (ib->hitBy[0] != 0 && ib->hitBy[1] != 0 && ib->hitBy[2] != 0)
                continue;
            if (!Hit_Check(e2, b))
                continue;
            res->flags |= 0x10;
            res->typeB = ie2->type;
            HitBox_SetHitRect2(e2, hb2);
            res->box = *(struct Box8 *)&gEwramData->hitRectLeft;
            if (ie2->onHit != NULL)
                ie2->onHit(e2, b, res);
            if (ib->onHurt != NULL)
                ib->onHurt(b, e2, res);
            break;
        }
    }
    if (res->flags & 1)
    {
        if (pi->onHurt != NULL)
            pi->onHurt(PLAYER, b, res);
        if (ib->onHit != NULL)
            ib->onHit(b, PLAYER, res);
    }
    return res->flags;
}
