/* Patrolling enemy (Entity_GetTargetDistanceX/BDEC distance checks, Tile_Classify probe); per-copy pinned bg->xPos reads keep the three tracking copies distinct. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"

extern void EntityMove(struct EwramData_EntityData *);
extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *);
extern s32 Entity_GetTargetDistanceY(struct EwramData_EntityData *);
extern s16 Target_GetPixelX(struct EwramData_EntityData *);
extern s32 Tile_Classify(s16, s16);

#define ANIM552 (*(u16 *)&entity->animStep)
static inline s32 Abs(s32 v)
{
    if (v < 0)
        v = -v;
    return v;
}

#define ANIMATE()                                                                 \
    if (entity->animScriptIndex == 2)                                                     \
    {                                                                             \
        if (ANIM552 == 0 && RandomNumberGenerator() % 3 == 0)              \
        {                                                                         \
            Anim_StartIndexed(entity, 0, 0, 0);                                        \
            entity->animDone = 0;                                                \
        }                                                                         \
    }                                                                             \
    else if (entity->animScriptIndex == 0 && entity->animDone)                           \
        Anim_StartIndexed(entity, 2, 0, 1);

#define TRACK0()                                                                   \
    {                                                                             \
        s32 a, b, t;                                                               \
        if (Abs((s16)Entity_GetTargetDistanceY(entity)) <= 0x10)                                     \
            break;                                                                \
        a = Entity_GetTargetDistanceX(entity);                                                 \
        b = Entity_GetTargetDistanceY(entity);                                                 \
        t = Abs((s16)a) + 8; if (Abs(t - Abs((s16)b)) > 0x1F)                                    \
            break;                                                                \
        if (entity->flipX)                                                        \
        {                                                                         \
            if ((s16)Entity_GetTargetDistanceX(entity) >= 0)                                        \
                break;                                                            \
            if (Tile_Classify(({ s32 t_ = entity->posX.part16.integer + ({ register u32 b_ asm("r2") = bg->xPos.part16.integer; b_; }); t_; }) + 0x15, bg->yPos.part16.integer + entity->posY.part16.integer - 9) != 2) entity->ext.wingedSkeleton.blocked = 1; /* FAKEMATCH: pinned register */ \
        }                                                                         \
        else                                                                      \
        {                                                                         \
            if ((s16)Entity_GetTargetDistanceX(entity) <= 0)                                        \
                break;                                                            \
            if (Tile_Classify(({ s32 t_ = entity->posX.part16.integer + (bg->xPos.part16.integer); t_; }) - 0x15, bg->yPos.part16.integer + entity->posY.part16.integer - 9) != 2) entity->ext.wingedSkeleton.blocked = 1; \
        }                                                                         \
    }

#define TRACK1()                                                                   \
    {                                                                             \
        s32 a, b, t;                                                               \
        if (Abs((s16)Entity_GetTargetDistanceY(entity)) <= 0x10)                                     \
            break;                                                                \
        a = Entity_GetTargetDistanceX(entity);                                                 \
        b = Entity_GetTargetDistanceY(entity);                                                 \
        t = Abs((s16)a) + 8; if (Abs(t - Abs((s16)b)) > 0x1F)                                    \
            break;                                                                \
        if (entity->flipX)                                                        \
        {                                                                         \
            if ((s16)Entity_GetTargetDistanceX(entity) >= 0)                                        \
                break;                                                            \
            if (Tile_Classify(({ s32 t_ = entity->posX.part16.integer + (bg->xPos.part16.integer); t_; }) + 0x15, bg->yPos.part16.integer + entity->posY.part16.integer - 9) != 2) entity->ext.wingedSkeleton.blocked = 1; \
        }                                                                         \
        else                                                                      \
        {                                                                         \
            if ((s16)Entity_GetTargetDistanceX(entity) <= 0)                                        \
                break;                                                            \
            if (Tile_Classify(({ s32 t_ = entity->posX.part16.integer + ({ register u32 b_ asm("r2") = bg->xPos.part16.integer; b_; }); t_; }) - 0x15, bg->yPos.part16.integer + entity->posY.part16.integer - 9) != 2) entity->ext.wingedSkeleton.blocked = 1; /* FAKEMATCH: pinned register */ \
        }                                                                         \
    }

#define TRACK2()                                                                   \
    {                                                                             \
        s32 a, b, t;                                                               \
        if (Abs((s16)Entity_GetTargetDistanceY(entity)) <= 0x10)                                     \
            break;                                                                \
        a = Entity_GetTargetDistanceX(entity);                                                 \
        b = Entity_GetTargetDistanceY(entity);                                                 \
        t = Abs((s16)a) + 8; if (Abs(t - Abs((s16)b)) > 0x1F)                                    \
            break;                                                                \
        if (entity->flipX)                                                        \
        {                                                                         \
            if ((s16)Entity_GetTargetDistanceX(entity) >= 0)                                        \
                break;                                                            \
            if (Tile_Classify(({ s32 t_ = entity->posX.part16.integer + (bg->xPos.part16.integer); t_; }) + 0x15, bg->yPos.part16.integer + entity->posY.part16.integer - 9) != 2) entity->ext.wingedSkeleton.blocked = 1; \
        }                                                                         \
        else                                                                      \
        {                                                                         \
            if ((s16)Entity_GetTargetDistanceX(entity) <= 0)                                        \
                break;                                                            \
            if (Tile_Classify(({ s32 t_ = entity->posX.part16.integer + ({ register u32 b_ asm("r2") = bg->xPos.part16.integer; b_; }); t_; }) - 0x15, bg->yPos.part16.integer + entity->posY.part16.integer - 9) != 2) entity->ext.wingedSkeleton.blocked = 1; /* FAKEMATCH: pinned register */ \
        }                                                                         \
    }

#define TURN(next)                                                                \
    {                                                                             \
        u16 anim = ANIM552;                                               \
        u8 f;                                                                     \
        if (anim != 0)                                                            \
            break;                                                                \
        f = *(ENTITY_RENDER_FLAGS_ADDRESS(entity)) & 0x40;                                     \
        if (f)                                                                    \
        {                                                                         \
            if ((s16)Entity_GetTargetDistanceX(entity) < 0)                                         \
            {                                                                     \
                entity->state = 1;                                              \
                entity->subState = next;                                           \
                break;                                                            \
            }                                                                     \
            entity->ext.wingedSkeleton.blocked = 0;                                \
        }                                                                         \
        else                                                                      \
        {                                                                         \
            if ((s16)Entity_GetTargetDistanceX(entity) > 0)                                         \
            {                                                                     \
                entity->state = 1;                                              \
                entity->subState = next;                                           \
                break;                                                            \
            }                                                                     \
            entity->ext.wingedSkeleton.blocked = 0;                                \
        }                                                                         \
    }

/**
 * @brief Flight state machine (state 0): fly across the view toward the target and
 * turn; start the throw (state 1) when the target is roughly diagonal.
 */
void WingedSkeleton_UpdateFly(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];

    switch (entity->subState)
    {
    case 0:
        EntityMove(entity);
        ANIMATE()
        if (ANIM552 == 0 && Abs((s16)Entity_GetTargetDistanceX(entity)) <= Abs((s16)Entity_GetTargetDistanceY(entity)) + 0x20)
        {
            entity->subState = 1;
            Anim_StartIndexed(entity, 3, 0, 0);
            entity->animDone = 0;
            break;
        }
        if (entity->ext.wingedSkeleton.blocked == 0)
            TRACK0()
        else
            TURN(10)
        break;
    case 1:
        if (entity->animDone)
        {
            Anim_StartIndexed(entity, 4, 0, 1);
            entity->subState++;
        }
        EntityMove(entity);
        break;
    case 2:
        EntityMove(entity);
        if (ANIM552 == 0)
        {
            if (entity->velX.whole > 0)
            {
                if (Target_GetPixelX(entity) - (s16)entity->posX.part16.integer >= -0x10)
                    goto track2;
            }
            else
            {
                if (Target_GetPixelX(entity) - (s16)entity->posX.part16.integer <= 0x10)
                    goto track2;
            }
            entity->subState = 3;
            Anim_StartIndexed(entity, 2, 0, 1);
            entity->animDone = 0;
            break;
        }
    track2:
        if (entity->ext.wingedSkeleton.blocked == 0)
            TRACK1()
        else
            TURN(0)
        break;
    case 3:
        EntityMove(entity);
        ANIMATE()
        if (entity->ext.wingedSkeleton.blocked == 0)
            TRACK2()
        else
            TURN(10)
        break;
    case 4:
        EntityMove(entity);
        break;
    }
}
