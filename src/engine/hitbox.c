/*
 * Hit boxes: overlap tests, attack / hurt checks, timers and the enable / disable helpers.
 */
#include "hitbox.h"
#include "core.h"
#include "gba.h"
#include "constants/main.h"
#include "data/data_0E0334.h"
#include "data/data_0E3464.h"
#include "structs/agb_sram.h"
#include "structs/ewram.h"
#include "structs/main.h"
#include "structs/entity_boxes.h"
#include "entity_hit.h"
#include "data/data_0E1CD0.h"

/**
 * @brief 41474 | Test whether the hitboxes of two entities overlap
 *
 * @return 1 on overlap, 0 otherwise (also when either hitbox is 1 pixel wide or high)
 */
u32 HitBox_Overlap(struct EwramData_EntityData *e1, struct SpriteBox *hb1, struct EwramData_EntityData *e2, struct SpriteBox *hb2)
{
    s32 l1, r1, l2, r2;

    if (hb1->width == 1)
        return 0;
    if (hb1->height == 1)
        return 0;
    if (hb2->width == 1)
        return 0;
    if (hb2->height == 1)
        return 0;

    if (!e1->flipX)
    {
        l1 = (s16)e1->posX.part16.integer + hb1->x;
        {
            s32 tmp = l1 - 1;
            r1 = tmp + hb1->width;
        }
    }
    else
    {
        r1 = (s16)e1->posX.part16.integer - hb1->x;
        {
            s32 tmp = r1 + 1;
            l1 = tmp - hb1->width;
        }
    }
    if (!e2->flipX)
    {
        l2 = (s16)e2->posX.part16.integer + hb2->x;
        {
            s32 tmp = l2 - 1;
            r2 = tmp + hb2->width;
        }
    }
    else
    {
        r2 = (s16)e2->posX.part16.integer - hb2->x;
        {
            s32 tmp = r2 + 1;
            l2 = tmp - hb2->width;
        }
    }
    if (l1 > r2)
        return 0;
    if (l2 > r1)
        return 0;

    if (!e1->flipY)
    {
        l1 = (s16)e1->posY.part16.integer + hb1->y;
        {
            s32 tmp = l1 - 1;
            r1 = tmp + hb1->height;
        }
    }
    else
    {
        r1 = (s16)e1->posY.part16.integer - hb1->y;
        {
            s32 tmp = r1 + 1;
            l1 = tmp - hb1->height;
        }
    }
    if (!e2->flipY)
    {
        l2 = (s16)e2->posY.part16.integer + hb2->y;
        {
            s32 tmp = l2 - 1;
            r2 = tmp + hb2->height;
        }
    }
    else
    {
        r2 = (s16)e2->posY.part16.integer - hb2->y;
        {
            s32 tmp = r2 + 1;
            l2 = tmp - hb2->height;
        }
    }
    if (l1 > r2)
        return 0;
    if (l2 <= r1)
        return 1;
    return 0;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   HitBox_SetHitRect2 = HitBox_SetHitRect */

/**
 * @brief 415A8 | Same as HitBox_SetHitRect.
 */
void HitBox_SetHitRect2(struct EwramData_EntityData *entity, struct SpriteBox *box)
{
    s32 tmp;

    // TODO: is tmp necessary? or perhaps inline?
    if (!entity->flipX)
    {
        gEwramData->hitRectLeft = box->x + entity->posX.part16.integer;
        tmp = box->width;
        gEwramData->hitRectRight = gEwramData->hitRectLeft - 1 + tmp;
    }
    else
    {
        gEwramData->hitRectRight = entity->posX.part16.integer - box->x;
        tmp =  box->width;
        gEwramData->hitRectLeft = gEwramData->hitRectRight + 1 - tmp;
    }

    if (!entity->flipY)
    {
        gEwramData->hitRectTop = box->y + entity->posY.part16.integer;
        tmp = box->height;
        gEwramData->hitRectBottom = gEwramData->hitRectTop - 1 + tmp;
    }
    else
    {
        gEwramData->hitRectBottom = entity->posY.part16.integer - box->y;
        tmp = box->height;
        gEwramData->hitRectTop = gEwramData->hitRectBottom + 1 - tmp;
    }
}

/**
 * @brief 41688 | Test whether an entity's hitbox overlaps the rectangle at EWRAM 0x13074
 *
 * @param entity Entity (position and flip flags)
 * @param box Hitbox (signed offset, width, height)
 * @return 1 on overlap, 0 otherwise (also when the rectangle or the hitbox is degenerate)
 */
u32 HitBox_OverlapsHitRect(struct EwramData_EntityData *entity, struct SpriteBox *box)
{
    s32 left;
    s32 right;
    s32 tmp;
    s32 top;
    s32 bottom;

    if ((s16)gEwramData->hitRectLeft == (s16)gEwramData->hitRectRight)
        return 0;
    if ((s16)gEwramData->hitRectTop == (s16)gEwramData->hitRectBottom)
        return 0;
    if (box->width == 1)
        return 0;
    if (box->height == 1)
        return 0;

    if (!entity->flipX)
    {
        tmp = (s16)entity->posX.part16.integer;
        top = box->x;
        left = tmp + top;
        right = left - 1;
        tmp = box->width;
        right += tmp;
    }
    else
    {
        right = (s16)entity->posX.part16.integer - box->x;
        left = right + 1;
        tmp = box->width;
        left -= tmp;
    }
    if ((s16)gEwramData->hitRectLeft > right)
        return 0;
    if (left > (s16)gEwramData->hitRectRight)
        return 0;

    if (!entity->flipY)
    {
        top = (s16)entity->posY.part16.integer + box->y;
        bottom = top - 1;
        tmp = box->height;
        bottom += tmp;
    }
    else
    {
        bottom = (s16)entity->posY.part16.integer - box->y;
        top = bottom + 1;
        tmp = box->height;
        top -= tmp;
    }
    if ((s16)gEwramData->hitRectTop > bottom)
        return 0;
    if (top <= (s16)gEwramData->hitRectBottom)
        return 1;
    return 0;
}

struct Rect16 { s16 left; s16 top; s16 right; s16 bottom; };

u32 HitBox_OverlapsRect(struct Rect16 *rect, struct EwramData_EntityData *entity, struct SpriteBox *box)
{
    s32 left;
    s32 right;
    s32 top;
    s32 bottom;
    s32 tmp;

    if (rect->left == rect->right)
        return 0;
    if (rect->top == rect->bottom)
        return 0;
    if (box->width == 1)
        return 0;
    if (box->height == 1)
        return 0;

    if (!entity->flipX)
    {
        tmp = (s16)entity->posX.part16.integer;
        top = box->x;
        left = tmp + top;
        right = left - 1;
        tmp = box->width;
        right += tmp;
    }
    else
    {
        right = (s16)entity->posX.part16.integer - box->x;
        left = right + 1;
        tmp = box->width;
        left -= tmp;
    }
    if (rect->left > right)
        return 0;
    if (left > rect->right)
        return 0;

    if (!entity->flipY)
    {
        top = (s16)entity->posY.part16.integer + box->y;
        bottom = top - 1;
        tmp = box->height;
        bottom += tmp;
    }
    else
    {
        bottom = (s16)entity->posY.part16.integer - box->y;
        top = bottom + 1;
        tmp = box->height;
        top -= tmp;
    }
    if (rect->top > bottom)
        return 0;
    if (top <= rect->bottom)
        return 1;
    return 0;
}

static inline u8 GetVal(s32 type)
{
    switch (type)
    {
    case 3:
        return gEwramData->hitTimerOverride[0];
    case 5:
        return gEwramData->hitTimerOverride[1];
    case 6:
        return gEwramData->hitTimerOverride[2];
    case 9:
        return gEwramData->hitTimerOverride[3];
    default:
        return sHitTimers[type];
    }
}

static inline void AddHit(u8 *dst, u8 *src)
{
    if (dst[4] == 0)
    {
        dst[4] = src[0];
        dst[8] = GetVal(src[0]) + 1;
    }
    else if (dst[5] == 0)
    {
        dst[5] = src[0];
        dst[9] = GetVal(src[0]) + 1;
    }
    else if (dst[6] == 0)
    {
        dst[6] = src[0];
        dst[10] = GetVal(src[0]) + 1;
    }
}

u32 Hit_Check(u8 *a, u8 *b)
{
    u8 *src;
    u8 *dst;
    struct EntityBoxes *hb;
    struct EntityBoxes *hb2;

    src = a + 0x70;
    dst = b + 0x70;
    hb = &gEntityBoxes[a[0x73]];
    hb2 = &gEntityBoxes[b[0x73]];
    HitBox_SetHitRect2((struct EwramData_EntityData *)b, (struct SpriteBox *)&hb2->body);
    if (HitBox_OverlapsHitRect((struct EwramData_EntityData *)a, (struct SpriteBox *)&hb->attack))
    {
        AddHit(dst, src);
        return 1;
    }
    if (src[1] == 0 && HitBox_OverlapsHitRect((struct EwramData_EntityData *)a, (struct SpriteBox *)&hb->body))
    {
        AddHit(dst, src);
        return 2;
    }
    return 0;
}

u32 Hit_CheckPlayerAttack(u8 *e)
{
    u8 *src;
    u8 *dst;
    void *hb;

    src = (u8 *)gEwramData->playerState.player + 0x70;
    dst = e + 0x70;
    hb = &gEntityBoxes[e[0x73]].body;
    if (HitBox_OverlapsRect((void *)0x03001A48, (struct EwramData_EntityData *)e, hb))
    {
        if (dst[4] == 0)
        {
            dst[4] = src[0];
            dst[8] = GetVal(src[0]) + 1;
        }
        else if (dst[5] == 0)
        {
            dst[5] = src[0];
            dst[9] = GetVal(src[0]) + 1;
        }
        else if (dst[6] == 0)
        {
            dst[6] = src[0];
            dst[10] = GetVal(src[0]) + 1;
        }
        return 1;
    }
    return 0;
}

u32 Hit_CheckPlayerHurt(u8 *e)
{
    u8 *dst;
    u8 *src;
    struct EntityBoxes *hb;
    void *rect;

    dst = (u8 *)gEwramData->playerState.player + 0x70;
    src = e + 0x70;
    hb = &gEntityBoxes[e[0x73]];
    rect = (void *)0x03001A50;
    if (HitBox_OverlapsRect(rect, (struct EwramData_EntityData *)e, (struct SpriteBox *)&hb->attack))
    {
        AddHit(dst, src);
        return 1;
    }
    if (src[1] == 0 && HitBox_OverlapsRect(rect, (struct EwramData_EntityData *)e, (struct SpriteBox *)&hb->body))
    {
        AddHit(dst, src);
        return 2;
    }
    return 0;
}

/* HitBox_GetBoxes. */
/**
 * @brief 42030 | Copy the entity's attack and body boxes
 *
 * @param attack Destination of the attack box
 * @param body Destination of the body box
 * @param entity Entity
 * @return 1, or 0 when the hit block is inactive
 */
s32 HitBox_GetBoxes(struct SpriteBox *attack, struct SpriteBox *body, struct EwramData_EntityData *entity)
{
    struct SpriteBox *src;
    struct EntityHit *hit = &entity->hit;

    if (hit->flags & 1)
    {
        src = &gEntityBoxes[entity->hit.boxSlot].attack.box;
        attack->x = src->x;
        attack->y = src->y;
        attack->width = src->width;
        attack->height = src->height;
        src = &gEntityBoxes[entity->hit.boxSlot].body.box;
        body->x = src->x;
        body->y = src->y;
        body->width = src->width;
        body->height = src->height;
        return 1;
    }
    return 0;
}

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
                HitBox_SetHitRect2((struct EwramData_EntityData *)e, (struct SpriteBox *)hb0);
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
                HitBox_SetHitRect2((struct EwramData_EntityData *)e, (struct SpriteBox *)hb);
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
            HitBox_SetHitRect2((struct EwramData_EntityData *)e2, (struct SpriteBox *)hb2);
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

/* Player hitbox rectangles: writes the two attack/body boxes of the current
 * player entity (gEntityBoxes slot boxSlot) into the IWRAM rectangles at
 * 0x03001A48 and 0x03001A50, mirrored by flipX/flipY. */

#define RECT0 ((struct Rect16 *)0x03001A48)
#define RECT1 ((struct Rect16 *)0x03001A50)

void HitBox_UpdatePlayerRects(void)
{
    struct EwramData_EntityData *p;
    struct SpriteBox *b0;
    struct SpriteBox *b1;
    struct Rect16 *r;
    struct Rect16 *q;

    p = gEwramData->playerState.player;
    b0 = &gEntityBoxes[p->hit.boxSlot].attack.box;
    b1 = &gEntityBoxes[p->hit.boxSlot].body.box;
    if (!p->flipX)
    {
        RECT0->left = b0->x + p->posX.part16.integer;
        RECT0->right = RECT0->left + b0->width;
        r = RECT0;
    }
    else
    {
        RECT0->right = p->posX.part16.integer - b0->x;
        RECT0->left = RECT0->right - b0->width;
        r = RECT0;
    }
    if (!p->flipY)
    {
        r->top = b0->y + p->posY.part16.integer;
        r->bottom = r->top + b0->height;
    }
    else
    {
        r->bottom = p->posY.part16.integer - b0->y;
        r->top = r->bottom - b0->height;
    }
    if (!p->flipX)
    {
        RECT1->left = b1->x + p->posX.part16.integer;
        RECT1->right = RECT1->left + b1->width;
        q = RECT1;
    }
    else
    {
        RECT1->right = p->posX.part16.integer - b1->x;
        RECT1->left = RECT1->right - b1->width;
        q = RECT1;
    }
    if (!p->flipY)
    {
        q->top = b1->y + p->posY.part16.integer;
        q->bottom = q->top + b1->height;
    }
    else
    {
        q->bottom = p->posY.part16.integer - b1->y;
        q->top = q->bottom - b1->height;
    }
}

/* Entity box-slot helpers 0x080426B0..0x0804291F (no upstream C existed). */
/**
 * @brief 426B0 | Count down the three hit timers of an entity
 *
 * A recent hit (hitBy[i]) is forgotten when its timer reaches zero. Only
 * while the entity's hit block is active (flags bit 0).
 *
 * @param entity Entity
 */
void Hit_UpdateTimers(struct EwramData_EntityData *entity)
{
    struct EntityHit *hit = &entity->hit;

    if (hit->flags & 1)
    {
        if (hit->hitTimer[0] != 0)
        {
            if (--hit->hitTimer[0] == 0)
                hit->hitBy[0] = 0;
        }
        if (hit->hitTimer[1] != 0)
        {
            if (--hit->hitTimer[1] == 0)
                hit->hitBy[1] = 0;
        }
        if (hit->hitTimer[2] != 0)
        {
            if (--hit->hitTimer[2] == 0)
                hit->hitBy[2] = 0;
        }
    }
}

/**
 * @brief 42700 | Record an attack type in hitBy[] (or restart its entry)
 *
 * @param entity Entity that was hit
 * @param type Attack type
 * @return 1 on success, 0 when all three entries hold other types
 */
s32 Hit_AddHitBy(struct EwramData_EntityData *entity, s32 type)
{
    struct EntityHit *hit = &entity->hit;

    if (hit->hitBy[0] == type)
    {
        hit->hitTimer[0] = 0;
    }
    else if (hit->hitBy[1] == type)
    {
        hit->hitTimer[1] = 0;
    }
    else if (hit->hitBy[2] == type)
    {
        hit->hitTimer[2] = 0;
    }
    else if (hit->hitBy[0] == 0)
    {
        hit->hitBy[0] = type;
        hit->hitTimer[0] = 0;
    }
    else if (hit->hitBy[1] == 0)
    {
        hit->hitBy[1] = type;
        hit->hitTimer[1] = 0;
    }
    else if (hit->hitBy[2] == 0)
    {
        hit->hitBy[2] = type;
        hit->hitTimer[2] = 0;
    }
    else
    {
        return 0;
    }
    return 1;
}

/**
 * @brief 42754 | Clear all entity boxes (gEntityBoxes)
 */
void HitBox_ClearAll(void)
{
    DMA_FILL_32(3, 0, gEntityBoxes, sizeof(gEntityBoxes));
}

/**
 * @brief 4277C | Activate the entity's hit block; only its attack box hurts
 *
 * @param entity Entity
 * @param type Attack type
 * @param onHit Callback when this entity hits another one
 * @param onHurt Callback when this entity is hit (a function address)
 * @return Always 1
 */
s32 Hit_Enable(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt)
{
    struct EntityHit *hit = &entity->hit;

    hit->type = type;
    hit->noContactDamage = 1;
    hit->flags = 1;
    hit->boxSlot = EntityGetSlot(entity);
    hit->hitBy[2] = 0;
    hit->hitBy[1] = 0;
    hit->hitBy[0] = 0;
    hit->hitTimer[2] = 0;
    hit->hitTimer[1] = 0;
    hit->hitTimer[0] = 0;
    hit->onHit = onHit;
    hit->onHurt = (void *)onHurt;
    return 1;
}

/**
 * @brief 427B8 | Activate the entity's hit block; its body also hurts (contact damage)
 *
 * @param entity Entity
 * @param type Attack type
 * @param onHit Callback when this entity hits another one
 * @param onHurt Callback when this entity is hit (a function address)
 * @return Always 1
 */
s32 Hit_EnableContact(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt)
{
    struct EntityHit *hit = &entity->hit;

    hit->type = type;
    hit->noContactDamage = 0;
    hit->flags = 1;
    hit->boxSlot = EntityGetSlot(entity);
    hit->hitBy[2] = 0;
    hit->hitBy[1] = 0;
    hit->hitBy[0] = 0;
    hit->hitTimer[2] = 0;
    hit->hitTimer[1] = 0;
    hit->hitTimer[0] = 0;
    hit->onHit = onHit;
    hit->onHurt = (void *)onHurt;
    return 1;
}

/**
 * @brief 427F4 | Like Hit_EnableContact with an extra unused argument
 *
 * @param entity Entity
 * @param type Attack type
 * @param unused Ignored
 * @param onHit Callback when this entity hits another one
 * @param onHurt Callback when this entity is hit
 * @return Always 1
 */
s32 Hit_EnableContactEx(struct EwramData_EntityData *entity, s32 type, s32 unused, void *onHit, s32 onHurt)
{
    struct EntityHit *hit = &entity->hit;

    hit->type = type;
    hit->noContactDamage = 0;
    hit->flags = 1;
    hit->boxSlot = EntityGetSlot(entity);
    hit->hitBy[2] = 0;
    hit->hitBy[1] = 0;
    hit->hitBy[0] = 0;
    hit->hitTimer[2] = 0;
    hit->hitTimer[1] = 0;
    hit->hitTimer[0] = 0;
    hit->onHit = onHit;
    hit->onHurt = (void *)onHurt;
    return 1;
}

/**
 * @brief 42828 | Deactivate the entity's hit block
 *
 * @param entity Entity
 */
void Hit_Disable(struct EwramData_EntityData *entity)
{
    struct EntityHit *hit = &entity->hit;

    hit->noContactDamage = 0;
    hit->flags = 0;
    hit->boxSlot = 0xFF;
    hit->hitBy[2] = 0;
    hit->hitBy[1] = 0;
    hit->hitBy[0] = 0;
    hit->hitTimer[2] = 0;
    hit->hitTimer[1] = 0;
    hit->hitTimer[0] = 0;
    hit->onHit = NULL;
    hit->onHurt = NULL;
}

/**
 * @brief 42848 | Set the entity's attack box
 *
 * @param entity Entity
 * @param box SpriteBox (byte aligned, copied byte by byte)
 * @return 0 when the hit block is inactive, otherwise 1
 */
s32 HitBox_SetAttack(struct EwramData_EntityData *entity, void *box)
{
    struct EntityHit *hit = &entity->hit;

    if (hit->flags & 1)
    {
        struct EntityBoxes *dst = &gEntityBoxes[entity->hit.boxSlot];
        struct SpriteBox *src = box;

        // Byte-wise copy (the record has byte alignment).
        dst->attack.box.x = src->x;
        dst->attack.box.y = src->y;
        dst->attack.box.width = src->width;
        dst->attack.box.height = src->height;
        return 1;
    }
    return 0;
}

/**
 * @brief 42884 | Set the entity's attack box from a packed 32-bit SpriteBox
 *
 * @param entity Entity
 * @param box Packed box (x, y, width, height bytes)
 * @return 0 when the hit block is inactive, otherwise 1
 */
s32 HitBox_SetAttackRaw(struct EwramData_EntityData *entity, s32 box)
{
    struct EntityHit *hit = &entity->hit;

    if (hit->flags & 1)
    {
        struct EntityBoxes *dst = &gEntityBoxes[entity->hit.boxSlot];
        dst->attack.raw = box;
        return 1;
    }
    return 0;
}

/**
 * @brief 428B4 | Set the entity's body box
 *
 * @param entity Entity
 * @param box SpriteBox (byte aligned, copied byte by byte)
 * @return 0 when the hit block is inactive, otherwise 1
 */
s32 HitBox_SetBody(struct EwramData_EntityData *entity, void *box)
{
    struct EntityHit *hit = &entity->hit;

    if (hit->flags & 1)
    {
        struct SpriteBox *dst = &(gEntityBoxes + entity->hit.boxSlot)->body.box;
        struct SpriteBox *src = box;

        dst->x = src->x;
        dst->y = src->y;
        dst->width = src->width;
        dst->height = src->height;
        return 1;
    }
    return 0;
}

/**
 * @brief 428F0 | Set the entity's body box from a packed 32-bit SpriteBox
 *
 * @param entity Entity
 * @param box Packed box (x, y, width, height bytes)
 * @return 0 when the hit block is inactive, otherwise 1
 */
s32 HitBox_SetBodyRaw(struct EwramData_EntityData *entity, s32 box)
{
    struct EntityHit *hit = &entity->hit;

    if (hit->flags & 1)
    {
        union EntityBox *dst = &(gEntityBoxes + entity->hit.boxSlot)->body;

        dst->raw = box;
        return 1;
    }
    return 0;
}

/* Convert both active entity boxes into world-space rectangles. */
struct RectangleWords { u32 a, b; };
/**
 * @brief Unreferenced: when the entity hit is enabled, store the rectangles of
 * its two hit boxes in first/second and return 1, else 0.
 */
s32 HitBox_GetBothRects(struct RectangleWords *first, struct RectangleWords *second, struct EwramData_EntityData *entity)
{
    struct EntityHit *state = &entity->hit;
    if (state->flags & 1)
    {
        struct SpriteBox *boxes = (struct SpriteBox *)&gEntityBoxes[entity->hit.boxSlot];
        struct SpriteBox *secondBox = boxes + 1;
        HitBox_SetHitRect2(entity, boxes);
        *first = *(struct RectangleWords *)&gEwramData->hitRectLeft;
        HitBox_SetHitRect2(entity, secondBox);
        *second = *(struct RectangleWords *)&gEwramData->hitRectLeft;
        return 1;
    }
    return 0;
}

/* Clear a matching id and its timer from an entity's three box slots. */
/**
 * @brief Unreferenced: clear the hit record (attacker and timer) of the given
 * attacker in the entity's three slots; return 1 when found.
 */
s32 Hit_ClearHitBy(struct EwramData_EntityData *entity, s32 attacker)
{
    struct EntityHit *hit = &entity->hit;
    if (hit->hitBy[0] == attacker)
    {
        hit->hitBy[0] = 0;
        hit->hitTimer[0] = 0;
    }
    else if (hit->hitBy[1] == attacker)
    {
        hit->hitBy[1] = 0;
        hit->hitTimer[1] = 0;
    }
    else if (hit->hitBy[2] == attacker)
    {
        hit->hitBy[2] = 0;
        hit->hitTimer[2] = 0;
    }
    else
        return 0;
    return 1;
}
