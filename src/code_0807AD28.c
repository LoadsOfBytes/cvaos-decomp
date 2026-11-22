/* Zombie_UpdateEmergingWalkingSinking: matched (timer accessed as a field, block-local hit for the two
   Target_IsInRect calls, u8 parameter of BeginSinking). */
/* Zombie's emerging, walking and sinking states. */
#include "code_08009A0.h"
#include "zombie_entity.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern void EntityDampVelocityX(struct EwramData_EntityData *entity, s32 amount);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s16);
extern s32 Collision_IsSlopeTile(s16, s16);
extern s32 Target_IsInRect(s16, s16, s32, s32);
extern void Zombie_SpawnFallingDebris(struct EwramData_EntityData *, s32, s32);
extern void Zombie_SpawnTemporarySprite(struct EwramData_EntityData *, s32, s32);
static inline void WalkVelocity(struct EwramData_EntityData *entity)
{
    if (entity->flipX) entity->velX.whole = 0x20000;
    else entity->velX.whole = -0x20000;
    entity->velY.whole = 0x10000;
    entity->accelY.whole = 0x2800;
}
static inline void BeginSinking(struct EwramData_EntityData *entity, s32 x, s32 y, u8 zero)
{
    entity->subState++;
    entity->animFlags |= 2;
    entity->posX.whole = x;
    entity->posY.whole = y;
    entity->timer = zero;
}
/**
 * @brief Wait for emergence, walk with collision, then pause and sink before entering removal state 1.
 */
void Zombie_UpdateEmergingWalkingSinking(struct EwramData_EntityData *entity)
{
    struct BgLayerState *background = &gEwramData->bgInfo[1];
    switch (entity->subState)
    {
    case 0:
        if (entity->animDone)
        {
            entity->subState = 1;
            if (RandomNumberGenerator() & 1) Anim_StartIndexed(entity, 1, 0, 1);
            else Anim_StartIndexed(entity, 4, 0, 1);
            WalkVelocity(entity);
            ZOMBIE_TIMER(entity) = (RandomNumberGenerator() & 511) + 600;
        }
        break;
    case 1:
    {

        s32 previousX, previousY, impact;
        if (entity->animTimer == 0) WalkVelocity(entity);
        EntityDampVelocityX(entity, 4);
        previousX = entity->posX.whole;
        previousY = entity->posY.whole;
        impact = Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 8) & 3;
        if (impact)
        {
            BeginSinking(entity, previousX, previousY, 0);
            break;
        }
        {
        s32 tile = Collision_IsSlopeTile((s16)(background->xPos.part16.integer + entity->posX.part16.integer),
                             (s16)(background->yPos.part16.integer + entity->posY.part16.integer));
        if (tile)
        {
            BeginSinking(entity, previousX, previousY, impact);
            break;
        }
        if (ZOMBIE_TIMER(entity)-- <= 0)
        {
            BeginSinking(entity, previousX, previousY, tile);
            break;
        }
        }
        if (entity->animStep == 0)
        {
            if (ZOMBIE_DEBRIS_SPAWNED(entity) == 0)
            {
                ZOMBIE_DEBRIS_SPAWNED(entity) = 1;
                if (entity->animScriptIndex == 4)
                {
                    previousX = entity->flipX ? -0x90000 : 0x90000;
                    previousY = -0x180000;
                }
                else
                {
                    previousX = entity->flipX ? 0x90000 : -0x90000;
                    previousY = -0x160000;
                }
                previousX += entity->posX.whole;
                previousY += entity->posY.whole;
                Zombie_SpawnFallingDebris(entity, previousX, previousY);
            }
        }
        if (entity->animStep == 0 || entity->animStep == 2)
        {
            if ((s8)ZOMBIE_SPRITE_SPAWNED(entity) == 0)
            {
                ZOMBIE_SPRITE_SPAWNED(entity) = 1;
                previousX = entity->posX.whole;
                previousY = entity->posY.whole;
                Zombie_SpawnTemporarySprite(entity, previousX, previousY);
            }
        }
        if (entity->animStep == 1 || entity->animStep == 3)
        {
            ZOMBIE_DEBRIS_SPAWNED(entity) = 0;
            ZOMBIE_SPRITE_SPAWNED(entity) = 0;
        }
        {
            s32 hit;
            if (entity->flipX)
                hit = Target_IsInRect(entity->posX.part16.integer, (s16)(entity->posY.part16.integer - 32), 80, 64);
            else
                hit = Target_IsInRect((s16)(entity->posX.part16.integer - 80), (s16)(entity->posY.part16.integer - 32), 80, 64);
            if (!hit) break;
        }
        if (!(ZOMBIE_TIMER(entity) & 1)) entity->animTimer++;
        break;
    }
    case 2:
        entity->animFlags |= 2;
        if (entity->timer++ > 59)
        {
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->timer = 0;
            entity->subState++;
        }
        break;
    case 3:
        if (entity->animDone)
        {
            s32 zero = 0;
            entity->state = 1;
            entity->timer = 1;
            entity->unk_518.part16.hp = zero;
        }
        break;
    }
}
