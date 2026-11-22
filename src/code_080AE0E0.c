/* Enemy update: counted hops with random/target-based facing, knockback fall and despawn states. */
#include "code_08009A0.h"
#include "worm_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void Player_TrackEntity(struct EwramData_EntityData *);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *);
extern void Player_QueueEntity(struct EwramData_EntityData *entity, u8 collisionFlags);
extern s32 Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern void Worm_StepCrawl(struct EwramData_EntityData *entity, s32 direction, s32 distance, s32 blocked);
extern void Worm_FollowGround(struct EwramData_EntityData *entity);
extern s32 Worm_GetSlopeAngle(struct EwramData_EntityData *entity);
extern void Effect_SpawnRandom(s32 x, s32 y, s32 p2, s32 p3, s32 layer);
extern void ObjAffine_Free(s32 index);
extern void ObjAffine_Set(s32, s32, s32, u16, u16, u16);

static inline struct EwramData_EntityData *CurV(struct EwramData *ew) { return *(struct EwramData_EntityData * volatile *)&ew->playerState.enemyTarget; }
static inline struct EwramData_EntityData *Cur(struct EwramData *ew) { return ew->playerState.enemyTarget; }
void Worm_Update(struct EwramData_EntityData *entity)
{
    u8 buffer[16];
    s32 zero = 0;
    s32 angle = 0;
    u32 n;
    struct EwramData *ew;

    if (entity->state)
    {
        Player_TrackEntity(entity);
        if (Enemy_IsStunned(entity))
        {
            if (entity->state == 3)
                return;
            Player_QueueEntity(entity, 0);
            Hit_CheckWithPlayer(buffer, entity);
            return;
        }
        if (entity->state != 3)
            Player_QueueEntity(entity, 1);
    }
    switch (entity->state)
    {
    case 0:
        entity->state = 0;
    case 1:
        { register u32 raw_ asm("r0") = *(vu8 *)&WORM_TURN_TIMER(entity); u8 t_; n = raw_; t_ = raw_; /* FAKEMATCH: pinned register */
        if (n)
        {
            entity->state = 2;
            if (n == 4)
            {
                if (WORM_CRAWL_DIR(entity))
                    WORM_CRAWL_DIR(entity) = 0;
                else
                    WORM_CRAWL_DIR(entity) = 1;
            }
            WORM_TURN_TIMER(entity)=t_-1;
            break;
        } }
        if (!Target_IsInRect(entity->posX.part16.integer - 0xA0, entity->posY.part16.integer - 0x3C, 0x140, 0x78))
            WORM_CRAWL_DIR(entity) = RandomNumberGenerator() & 1;
        else if (!(ew = gEwramData)->playerState.enemyTarget)
            WORM_CRAWL_DIR(entity) = RandomNumberGenerator() & 1;
        else if (CurV(ew)->posX.whole < entity->posX.whole)
            WORM_CRAWL_DIR(entity) = n;
        else
            WORM_CRAWL_DIR(entity) = 1;
        entity->state = 2;
        break;
    case 2:
        Worm_StepCrawl(entity, WORM_CRAWL_DIR(entity), 0, 0);
        Worm_FollowGround(entity);
        if (!entity->subState)
            entity->state = 1;
        break;
    case 3:
        WORM_AGGRESSIVE(entity) = 1;
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x2000;
        if (entity->flipX)
            entity->accelY.whole -= 0x100;
        else
            entity->accelY.whole += 0x100;
        angle = entity->accelY.whole;
        if (!(entity->timer & 7))
            Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0x100000, 0x10000, entity->spriteLayer - 2);
        if (--entity->timer == 0)
        {
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
            return;
        }
        break;
    case 4:
        entity->timer = 2;
        entity->state = 5;
    case 5:
        if (--entity->timer == 0)
        {
            ObjAffine_Free(entity->affineIndex);
            EntityDelete(entity);
        }
        return;
    case 6:
        ObjAffine_Set(angle, 0x10000, zero + 0x10000, entity->flipX, 0, entity->affineIndex);
        Anim_Update(entity);
        return;
    }
    if (entity->state != 3)
    {
        Hit_CheckWithPlayer(buffer, entity);
        angle = Worm_GetSlopeAngle(entity);
        entity->accelY.whole = angle;
    }
    ObjAffine_Set(angle, 0x10000, zero + 0x10000, entity->flipX, 0, entity->affineIndex);
    Anim_Update(entity);
}
