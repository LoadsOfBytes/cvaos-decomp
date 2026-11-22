/* Chaos second form: head update (5A7C). */
#include "code_08009A0.h"
#include "chaos_entity.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
#include "code_080D73B8.h"
#include "syscalls.h"

extern void *memcpy(void *dst, const void *src, unsigned long size);
extern const u8 sUnk_08119264[];
extern const u16 sUnk_0811926C[];
extern const u16 sUnk_08119280[];
extern const u16 sUnk_081192A0[];
extern const u16 sUnk_081192C0[];
extern struct AnimScript sChaos2SparkAnimScript;

extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void EntityMove(struct EwramData_EntityData *);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *, s32);
extern s32 Entity_GetVisibilityPattern(s32);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);
extern void Heap_Free(void *);
extern void Chaos2SnakeHead_FireShot(struct EwramData_EntityData *, s32);

#define TARGET_X(e) (CHAOS_SNAKE_TARGET_X((e)))
#define TARGET_Y(e) (CHAOS_SNAKE_TARGET_Y((e)))
#define ANGLE(e) (*(u16 *)&CHAOS_SNAKE_ANGLE((e)))
#define TIMER(e) (*(u16 *)&CHAOS_SNAKE_TIMER((e)))
#define TURN(e) (CHAOS_SNAKE_TURN((e)))
#define MODE(e) (CHAOS_SNAKE_MODE((e)))
#define POINT(e) (CHAOS_SNAKE_POINT((e)))

/* Shift the 120-entry history (6 bytes each) by one entry. */
#define SHIFT_TRAIL(trail)                                  \
    {                                                       \
        s32 i_;                                             \
        for (i_ = 119; i_ > 0; i_--)                        \
        {                                                   \
            (trail)[i_ * 6 + 0] = (trail)[i_ * 6 - 6];      \
            (trail)[i_ * 6 + 1] = (trail)[i_ * 6 - 5];      \
            (trail)[i_ * 6 + 2] = (trail)[i_ * 6 - 4];      \
            (trail)[i_ * 6 + 3] = (trail)[i_ * 6 - 3];      \
            (trail)[i_ * 6 + 4] = (trail)[i_ * 6 - 2];      \
            (trail)[i_ * 6 + 5] = (trail)[i_ * 6 - 1];      \
        }                                                   \
    }

/* Pick a new waypoint 0-7 different from the current one: with probability
 * 1/3 per try one of the outer points 4-7, otherwise one of 0-3. */
#define PICK(e, r, L)                                               \
    do                                                              \
    {                                                               \
        while (RandomNumberGenerator() % 3 == 0)                    \
        {                                                           \
            r = (RandomNumberGenerator() & 3) + 4;                  \
            if (POINT(e) != r) { POINT(e) = r; goto L; }            \
        }                                                           \
        r = RandomNumberGenerator() & 3;                            \
    } while (POINT(e) == r);                                        \
    POINT(e) = r;                                                   \
    L:;

/**
 * @brief Snake head update: move along its path, record the 120-entry position
 * history for the segments and fire shots (Chaos2SnakeHead_FireShot); break
 * apart with the core.
 */
void Chaos2SnakeHead_Update(struct EwramData_EntityData *e)
{
    struct EwramData_EntityData *core = CHAOS_LINK(e);
    u8 unused[0x20];
    u8 collision[0x10];
    u8 t8[8];
    u16 t0[5][2];
    u16 t1[8][2];
    u16 t1b[16];
    u16 t2[8][3];
    u16 t3[8][3];
    u8 *trail;

    memcpy(t8, sUnk_08119264, 8);
    memcpy(t0, sUnk_0811926C, 0x14);
    memcpy(t1, sUnk_08119280, 0x20);
    memcpy(t1b, sUnk_081192A0, 0x20);
    memcpy(t2, sUnk_081192C0, 0x30);
    memcpy(t3, sUnk_081192C0, 0x30);
    trail = (u8 *)CHAOS_SNAKE_TRAIL(e);

    if (e->state == 0)
    {
        if (core != NULL && core->updateFunc != NULL
            && core->unk_518.part8.enemyId == e->unk_518.part8.enemyId)
        {
            switch (e->subState)
            {
            case 0:
                e->subState = 1;
                e->accelY.whole = 0x800;
            case 1:
                if (MODE(e) == 0)
                {
                    TARGET_X(e) = t0[POINT(e)][0];
                    TARGET_Y(e) = t0[POINT(e)][1];
                }
                else if (MODE(e) == 1)
                {
                    TARGET_X(e) = t1[POINT(e)][0];
                    TARGET_Y(e) = t1[POINT(e)][1];
                }
                else if (MODE(e) == 2)
                {
                    TARGET_X(e) = t2[POINT(e)][0];
                    TARGET_Y(e) = t2[POINT(e)][1];
                }
                else if (MODE(e) == 3)
                {
                    TARGET_X(e) = t3[POINT(e)][0];
                    TARGET_Y(e) = t3[POINT(e)][1];
                }
                e->subState++;
                e->timer = 0;
                TIMER(e) = 0;
                SHIFT_TRAIL(trail);
                trail[0] = 0;
                trail[1] = 0;
                trail[2] = e->animFrame;
                trail[3] = e->spriteLayer;
                trail[4] = (u16)ANGLE(e) >> 8;
                trail[5] = ANGLE(e);
                break;
            case 2:
            {
                s8 ox, oy;
                s32 dx, dy, dist;
                u16 diff;
                s32 r;

                dx = TARGET_X(e) - (s16)e->posX.part16.integer;
                dy = TARGET_Y(e) - e->posY.part16.integer;
                dx = dx * dx;
                dy = dy * dy;
                dist = Sqrt(dx + dy);
                if (dist <= 0)
                {
                    e->posX.part16.integer = TARGET_X(e);
                    e->posX.part16.fraction = 0;
                    e->posY.part16.integer = TARGET_Y(e);
                    e->posY.part16.fraction = 0;
                    e->accelX.whole = 0;
                }
                ox = e->posX.part16.integer;
                oy = e->posY.part16.integer;
                dx = TARGET_X(e) - (s16)e->posX.part16.integer;
                dy = TARGET_Y(e) - e->posY.part16.integer;
                { s32 a_ = ArcTan2(dx, dy); diff = (u16)(a_ - ANGLE(e)); }
                if (dist <= 5)
                {
                    /* FAKEMATCH: the target keeps two register copies of diff
                     * (cmp in r0, add in r1). */
                    s32 c_ = diff;
                    asm("" : "+r"(c_)); /* FAKEMATCH */
                    if (c_ > 0x20)
                        TURN(e) += 0x20;
                    else
                    {
                        s32 d_ = diff;
                        asm("" : "+r"(d_)); /* FAKEMATCH */
                        TURN(e) += d_;
                    }
                }
                else
                {
                    if (TURN(e) <= 0x27F)
                        TURN(e) += 8;
                    else
                        TURN(e) = 0x280;
                }
                if ((s16)diff >= 0)
                {
                    if (diff > TURN(e))
                        ANGLE(e) += TURN(e);
                    else if (diff > *(vu16 *)&TURN(e) >> 1)
                        ANGLE(e) = diff + ANGLE(e);
                }
                else
                {
                    if ((s32)diff < 0x10000 - TURN(e))
                        ANGLE(e) -= TURN(e);
                    else if ((s32)diff < 0x10000 - (TURN(e) >> 1))
                    {

                        ANGLE(e) = diff + ANGLE(e);
                    }
                }
                if (MODE(e) == 0)
                {
                    if (e->spriteLayer > 0x11)
                        e->spriteLayer--;
                    if (e->timer > 5)
                    {
                        if (e->animFrame == 10)
                            e->animFrame = 11;
                    }
                    else if (e->animFrame == 5)
                        e->animFrame = 10;
                }
                else if (MODE(e) == 1)
                {
                    if (e->timer > 9)
                    {
                        if (e->animFrame == 10)
                            e->animFrame = 5;
                    }
                    else if (e->animFrame == 5)
                        e->animFrame = 10;
                }
                else if (MODE(e) == 2)
                {
                    if (e->timer > 9)
                    {
                        if (e->animFrame == 10)
                            e->animFrame = 5;
                    }
                    else if (e->animFrame == 5)
                        e->animFrame = 10;
                }
                else if (MODE(e) == 3)
                {
                    if (e->timer > 9 && e->animFrame == 6)
                        e->animFrame = 5;
                }
                dx = Sin(ANGLE(e) + 0x4000);
                dy = Sin(ANGLE(e));
                if (e->animFrame != 11)
                    ObjAffine_Set(ANGLE(e) - 0x8000, 0x10000, 0x10000, e->flipX, e->flipY, e->affineIndex);
                else
                    ObjAffine_Set(0, 0x10000, 0x10000, e->flipX, e->flipY, e->affineIndex);
                if (dist > 0x28)
                {
                    if (e->accelX.whole <= 0x27FFF)
                        e->accelX.whole += e->accelY.whole * 2;
                }
                else if (dist > 0x20)
                {
                    if (e->accelX.whole > 0x20000)
                        e->accelX.whole -= e->accelY.whole * 3;
                }
                else if (dist > 0x18)
                {
                    if (e->accelX.whole > 0x18000)
                        e->accelX.whole -= e->accelY.whole * 2;
                }
                else if (dist > 0xC)
                {
                    if (e->accelX.whole > 0x10000)
                        e->accelX.whole -= e->accelY.whole * 2;
                }
                else if (dist > 4)
                {
                    if (e->accelX.whole > 0x8000)
                        e->accelX.whole -= e->accelY.whole * 2;
                }
                else if (e->accelX.whole > 0x4000)
                    e->accelX.whole -= e->accelY.whole * 2;
                else if (e->accelX.whole > 0x1000)
                    e->accelX.whole -= e->accelY.whole;
                else if (e->accelX.whole <= 0xFFF)
                    e->accelX.whole = 0x1000;
                dx = FixedMultiply(dx, e->accelX.whole);
                dy = FixedMultiply(dy, e->accelX.whole);
                e->velX.whole = dx;
                e->velY.whole = dy;
                EntityMove(e);
                e->timer++;
                TIMER(e)++;
                ox = (u8)e->posX.part16.integer - ({ s32 t_ = (s8)ox; t_; });
                oy = (u8)e->posY.part16.integer - ({ s32 t_ = (s8)oy; t_; });
                SHIFT_TRAIL(trail);
                trail[0] = ox;
                trail[1] = oy;
                trail[2] = e->animFrame;
                trail[3] = e->spriteLayer;
                trail[4] = (u16)ANGLE(e) >> 8;
                trail[5] = ANGLE(e);
                if (dist <= 2)
                {

                    if (MODE(e) == 0)
                    {
                        e->subState = 1;
                        MODE(e) = 2;
                        do
                            r = RandomNumberGenerator() & 7;
                        while (POINT(e) == r);
                        POINT(e) = r;
                        e->animFrame = 5;
                    }
                    else if (MODE(e) == 1)
                    {
                        e->subState = 3;
                        e->timer = 0;
                        e->stage = 0;
                    }
                    else if (MODE(e) == 2)
                    {
                        e->subState = 1;
                        e->spriteLayer = 0x19;
                        e->hit.flags |= 2;
                        e->hit.flags |= 4;
                        MODE(e) = 3;
                        PICK(e, r, pk1);
                    }
                    else if (MODE(e) == 3)
                    {
                        e->subState = 1;
                        if (POINT(e) <= 3 && (RandomNumberGenerator() & 0x7F) <= 0x20)
                        {
                            e->spriteLayer = 0x11;
                            e->hit.flags &= ~2;
                            e->hit.flags &= ~4;
                            MODE(e) = 0;
                        }
                        else if ((RandomNumberGenerator() & 0x7F) <= 0x40)
                        {
                            e->spriteLayer = 0x11;
                            e->hit.flags &= ~2;
                            e->hit.flags &= ~4;
                            MODE(e) = 1;
                            PICK(e, r, pk2);
                        }
                        else
                        {
                            e->spriteLayer = 0x11;
                            e->hit.flags &= ~2;
                            e->hit.flags &= ~4;
                            MODE(e) = 2;
                            /* Same as PICK, written with gotos: the outer
                             * retry must jump straight to the inner test so
                             * that cross-jumping keeps the outer compare. */
                            {
                                s32 m_ = 3;

                                goto t3;
                            b3:
                                r = (RandomNumberGenerator() & m_) + 4;
                                if (POINT(e) != r)
                                    goto pk3;
                            t3:
                                if (RandomNumberGenerator() % 3 == 0)
                                    goto b3;
                                r = RandomNumberGenerator() & m_;
                                if (POINT(e) == r)
                                    goto t3;
                            }
                        pk3:
                            POINT(e) = r;
                        }
                    }
                    TURN(e) = 0;
                }
                else if (TIMER(e) > 0x12B)
                {

                    e->subState = 1;
                    do
                        r = RandomNumberGenerator() & 3;
                    while (POINT(e) == r);
                    POINT(e) = r;
                }
                if (core->state == 3)
                {
                    e->state = 10;
                    e->subState = 0;
                    e->velX.whole = 0;
                    e->velY.whole = 0;
                    e->accelX.whole = 0;
                    e->accelY.whole = 0x2000;
                    e->timer = 0x28;
                }
                break;
            }
            case 3:
                switch (e->stage)
                {
                case 0:
                    Anim_Start(e, (struct AnimScript *)((u8 *)&sChaos2SparkAnimScript + 0x14), 2, 0);
                    e->stage++;
                    PlaySong(0x77);
                case 1:
                    if (*(u16 *)&e->animStep == 9)
                    {
                        Chaos2SnakeHead_FireShot(e, ANGLE(e));
                        PlaySong(0x147);
                    }
                    if (e->animDone)
                    {
                        MODE(e) = 2;
                        e->subState = 1;
                        e->stage = 0;
                        POINT(e) = (RandomNumberGenerator() % 4) + 4;
                    }
                    break;
                }
                Anim_Update(e);
                SHIFT_TRAIL(trail);
                trail[0] = 0;
                trail[1] = 0;
                trail[2] = 5;
                trail[3] = e->spriteLayer;
                trail[4] = (u16)ANGLE(e) >> 8;
                trail[5] = ANGLE(e);
                break;
            }
            Hit_CheckWithPlayer(collision, e);
        }
        else
        {
            e->state = 10;
            e->subState = 0;
            e->unk_518.part16.hp = 0;
            e->palette = e->unk_510.part8.basePalette;
            e->timer = 0x28;
            e->unk_510.part8.hitFlashTimer = 0;
            e->accelX.whole = 0;
            e->accelY.whole = 0x2000;
        }
    }
    else if (e->state == 10)
    {
        if ((e->timer & 3) == 0)
            Effect_SpawnRandom(e->posX.whole, e->posY.whole - 0x60000, 0x100000, 0xC0000, e->spriteLayer - 1);
        EntityAccelerateAndMove(e, 0x70000);
        e->hidden = Entity_GetVisibilityPattern(e->timer);
        if ((s8)--e->timer <= 0)
        {
            if (CHAOS_SNAKE_TRAIL(e) != 0)
                Heap_Free((void *)CHAOS_SNAKE_TRAIL(e));
            ObjAffine_Free(e->affineIndex);
            EntityDelete(e);
        }
    }
}
