/* Persephone_UpdateAttack: enemy behaviour states; matched. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *, s32);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s16 Entity_GetTargetDistanceX(struct EwramData_EntityData *);
extern void Persephone_SpawnAttackBox(struct EwramData_EntityData *, s32, s32);
extern void EntityDampVelocityX(struct EwramData_EntityData *entity, s32 factor);

/**
 * @brief Attack state machine (state 1): approach (animation 4, sound 0x1CA) and
 * swing, spawning a one-frame hit box in front (Persephone_SpawnAttackBox,
 * sound 0x85); then back to state 0.
 */
void Persephone_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 d;
    s32 v;
    s32 found;
    s32 r;
    s32 res;
    s32 speed;
    s32 speed3;

    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 4, 0, 0);
        entity->subState++;
        entity->ext.persephone.contactHit = 0;
        entity->flipX = Entity_IsTargetRight(entity);
        v = Entity_GetTargetDistanceX(entity);
        if (v < 0)
            v = -v;
        if (v > 0x20 && !(RandomNumberGenerator() & 1))
        {
            entity->subState = 10;
            break;
        }
    case 1:
        if (entity->animStep > 1)
        {
            PlaySong(0x1CA);
            entity->subState++;
            entity->timer++;
            v = Entity_GetTargetDistanceX(entity);
            if (v < 0)
                v = -v;
            if (v <= 7)
                v = -0x10000;
            else if (v <= 0xF)
                v = -0x8000;
            else if (v <= 0x1F)
                v = 0x10000;
            else if (v <= 0x2F)
                v = 0x34000;
            else
                v = 0x40000;
            if (entity->flipX)
                entity->velX.whole = v;
            else
                entity->velX.whole = -v;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
        }
        break;
    case 2:
        if (entity->animStep > 4)
            entity->velX.whole = 0;
        EntityDampVelocityX(entity, 8);
        speed = 0x10000;
        res = Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 8);
        if ((res & 4) && entity->animDone)
        {
            if (entity->timer > 1 || (s8)entity->ext.persephone.contactHit == 1)
                goto land;
            goto idle;
        }
        if (*(u16 *)&entity->animStep == 3)
        {
            Persephone_SpawnAttackBox(entity, entity->posX.whole, entity->posY.whole);
            PlaySong(0x85);
        }
        break;
    case 3:
        entity->flipX = Entity_IsTargetRight(entity);
        if (entity->flipX)
            entity->velX.whole = -0x8000;
        else
            entity->velX.whole = 0x8000;
        speed3 = 0x10000;
        Entity_MoveWithCollision(entity, speed3, -0x200000, 0x80000, 8);
        found = Target_IsInRect(entity->posX.part16.integer - 0x3C, entity->posY.part16.integer - 0x30, 0x78, 0x60);
        if (!found)
        {
            entity->state = found;
            entity->subState = 5;
            entity->timer = found;
            goto move3;
        }
        r = Entity_ClampXToRoom(entity, 1);
        if (r)
        {
            entity->state = 0;
            entity->subState = 5;
            do { entity->timer = 0; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
            goto move3;
        }
        if (entity->timer++ <= 0x3B)
            break;
        entity->state = r;
        entity->subState = 6;
        entity->timer = r;
    move3:
        entity->velY.whole = speed3;
        entity->accelY.whole = 0x2800;
        break;
    case 10:
        if (entity->animStep)
        {
            PlaySong(0x1CA);
            entity->subState++;
            entity->timer++;
            v = 0x14000;
            if (entity->flipX)
                entity->velX.whole = v;
            else
                entity->velX.whole = -v;
            entity->velY.whole = -0x20000;
            entity->accelY.whole = 0x2000;
            Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 8);
        }
        break;
    case 11:
        if (entity->animStep > 4)
            entity->animFlags |= 2;
        speed = 0x10000;
        res = Entity_MoveWithCollision(entity, speed, -0x200000, 0x80000, 8);
        if (res & 4)
        {
            if (entity->timer > 1 || (s8)entity->ext.persephone.contactHit == 1)
            {
            land:
                entity->subState = 3;
                entity->timer = 0;
                entity->velY.whole = speed;
                entity->accelY.whole = 0x2800;
                Anim_StartIndexed(entity, 5, 0, 1);
                break;
            }
        idle:
            entity->subState = 0;
            break;
        }
        if (*(u16 *)&entity->animStep == 3)
        {
            Persephone_SpawnAttackBox(entity, entity->posX.whole, entity->posY.whole);
            PlaySong(0x85);
        }
        break;
    }
}
