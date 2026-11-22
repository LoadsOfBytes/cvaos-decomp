/* Witch_UpdateCast. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Entity_CreateObject(u32 *update);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 Target_GetPixelX(struct EwramData_EntityData *entity);
extern s32 Target_IsNearEntity(struct EwramData_EntityData *, s32, s32);
extern void EntityDampVelocityX(struct EwramData_EntityData *entity, s32 factor);
extern void EntityDampVelocityY(struct EwramData_EntityData *entity, s32 factor);
extern void EntityMoveX(struct EwramData_EntityData *entity);
extern void EntityMoveY(struct EwramData_EntityData *entity);
extern void Witch_SpawnOrbitParticles(struct EwramData_EntityData *entity, s16 phase);
extern void WitchCastEffect_Update(struct EwramData_EntityData *entity);
extern void WitchSpell_Update(struct EwramData_EntityData *entity);
extern void Witch_SpawnMovementParticle(struct EwramData_EntityData *entity);

static inline s32 ABS(s32 x)
{
    if (x < 0)
        x = -x;
    return x;
}
static inline u8 Near(s32 dx, s32 dy)
{
    u8 r;
    if (ABS(dx) <= 0x3FFFF && ABS(dy) <= 0x3FFFF)
        r = 1;
    else
        r = 0;
    return r;
}
#define ABSM(x) ((x) < 0 ? -(x) : (x))
#define TIMER entity->unk_4FC.part16.unk_4FC
#define STEER(f, d) \
    if ((d) > 0x40000) \
        f = 0x1000; \
    else if ((d) < -0x40000) \
        f = -0x1000; \
    else \
        f = 0;

static inline s32 Home(struct EwramData_EntityData *entity, s32 dx, s32 dy)
{
    STEER(entity->accelX.whole, dx)
    STEER(entity->accelY.whole, dy)
    EntityDampVelocityX(entity, 0x20);
    EntityDampVelocityY(entity, 0x20);
    EntityAccelerateAndMove(entity, 0x28000);
    if (({ register s32 t asm("r2") = dx; if (t < 0) t = -t; t; }) <= 0x3FFFF && ABSM(dy) <= 0x3FFFF) /* FAKEMATCH: pinned register */
        return 1;
    return 0;
}

static inline u8 PlayerIsRight(struct EwramData_EntityData *e)
{
    s16 *x = &e->posX.part16.integer;
    if (*x >= (s16)Target_GetPixelX(e))
        return 0;
    return 1;
}

#define SPAWN(func) \
    child = Entity_CreateObject((u32 *)func); \
    if (child != NULL) \
    { \
        child->unk_50C.part8.unk_50C = entity->unk_4F4.part8.unk_4F4; \
        child->scrollWithBg = 1; \
        child->flipX = entity->flipX; \
        child->attack = entity->attack; \
    }

/**
 * @brief Cast state (1): approach above the target, turn, pause, then create
 * the attached effect, orbit particles and spell before returning to wait.
 * Keep the pinned register in the Home helper required by the matching source.
 */
void Witch_UpdateCast(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;
    s32 dx, dy;

    switch (entity->subState)
    {
    case 0:
        TIMER = 0;
        entity->velX.whole = 0;
        entity->velY.whole = 0;
        entity->subState = 1;
    case 1:
        TIMER++;
        dx = (-(Entity_IsTargetRight(entity) << 6) + 0x20) << 16;
        dy = (Sin(TIMER << 9) << 3) - 0x300000;
        entity->unk_500.whole = Target_GetX(entity) + dx;
        entity->unk_504.whole = Target_GetY(entity) + dy;
        dx = entity->unk_500.whole - entity->posX.whole;
        dy = entity->unk_504.whole - entity->posY.whole;
        if (Home(entity, dx, dy) || Target_IsNearEntity(entity, 0x50, 0x36))
        {
            entity->timer = 0x20;
            entity->subState = 3;
            entity->stage = 0;
            break;
        }
        if (entity->flipX != Entity_IsTargetRight(entity) && ABSM((s16)Target_GetPixelX(entity) - (s16)entity->posX.part16.integer) > 8)
        {
            entity->flipX ^= 1;
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->subState = 2;
            break;
        }
        if (TIMER & 1)
            Witch_SpawnMovementParticle(entity);
        break;
    case 2:
        dx = (-(Entity_IsTargetRight(entity) << 6) + 0x20) << 16;
        entity->unk_500.whole = Target_GetX(entity) + dx;
        entity->unk_504.whole = Target_GetY(entity) - 0x380000;
        dx = entity->unk_500.whole - entity->posX.whole;
        Home(entity, dx, entity->unk_504.whole - entity->posY.whole);
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState = 1;
        }
        break;
    case 3:
        EntityDampVelocityX(entity, 0x40);
        EntityDampVelocityY(entity, 0x40);
        EntityMoveX(entity);
        EntityMoveY(entity);
        if (entity->stage == 0 && entity->timer > 8)
        {
            if (entity->flipX != PlayerIsRight(entity))
            {
                entity->flipX ^= 1;
                Anim_StartIndexed(entity, 1, 0, 0);
                entity->stage = 1;
            }
        }
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->subState = 4;
        break;
    case 4:
        if (entity->animFlags & 4)
        {
            entity->timer = 0x40;
            entity->subState = 5;
            Anim_StartIndexed(entity, 3, 0, 1);
            *(u8 *)&entity->unk_4F8 |= 2;
            SPAWN(WitchCastEffect_Update);
            PlaySong(0x81);
        }
        break;
    case 5:
        if (entity->timer != 0)
        {
        if (entity->timer > 0x30)
            Witch_SpawnOrbitParticles(entity, ((entity->timer << 10) + (RandomNumberGenerator() & 0xF)) << 4);
        if (entity->timer == 0x20)
        {
            SPAWN(WitchSpell_Update);
        }
        entity->timer--;
        break;
        }
        *(u8 *)&entity->unk_4F8 &= 0xFD;
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->subState = 6;
        break;
    case 6:
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->state = 0;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}
