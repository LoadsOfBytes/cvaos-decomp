#include "code_08009A0.h"
#include "succubus_entity.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Entity_ClampXToRoom(struct EwramData_EntityData *, s32);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);
extern s32 Player_StartScriptedAnimation(s32, s32, s32, s32);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);

/**
 * @brief Attack state machine (state 1): dash at 3 px per frame (sound 0x1C6) in
 * grab mode (unk_500 = 1); when Succubus_DamagePlayerOnContact latched on
 * (unk_501) hold the player (sub-states 10.., sound 0x7B), then let go
 * (unk_501 = unk_4FF = 0) and return to state 0.
 */
void Succubus_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 speed;
    s32 speed2;
    s32 hit;
    s32 dx;
    s32 v;

    switch (entity->subState)
    {
    case 0:
        EntityDampVelocityX(entity, 0x10);
        Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x40000, 8);
        Entity_ClampXToRoom(entity, 4);
        if (entity->timer++ > 0x27)
        {
            Anim_StartIndexed(entity, 2, 0, 0);
            entity->subState++;
            entity->velX.whole = 0;
            entity->timer = 0;
            PlaySong(0x1C6);
        }
        break;
    case 1:
        entity->flipX = Entity_IsTargetRight(entity);
        EntityDampVelocityX(entity, 0x18);
        Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x40000, 8);
        Entity_ClampXToRoom(entity, 4);
        if (entity->animStep > 2)
        {
            if (entity->flipX)
                entity->velX.whole = 0x30000;
            else
                entity->velX.whole = -0x30000;
            entity->subState++;
            entity->timer = 0;
            SUCCUBUS_LUNGING(entity) = 1;
            SUCCUBUS_GRABBED(entity) = 0;
            Afterimage_Create(entity, 0x38, 0xC, 0x10);
            Afterimage_Create(entity, 0x38, 6, 0x10);
        }
        break;
    case 2:
        EntityDampVelocityX(entity, 0x18);
        speed = 0x10000;
        Entity_MoveWithCollision(entity, speed, -0x200000, 0x40000, 8);
        Entity_ClampXToRoom(entity, 4);
        hit = *(s8 *)&SUCCUBUS_GRABBED(entity);
        if (hit)
        {
            entity->subState = 10;
            entity->timer = 0x10;
            SUCCUBUS_DRAIN_COUNT(entity) = 0;
            SUCCUBUS_DRAIN_TIMER(entity) = 0x14;
            SUCCUBUS_ESCAPE_PRESSES(entity) = 0;
            break;
        }
        if (entity->timer++ > 0x1D)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->state = hit;
            entity->subState = 2;
            entity->timer = hit;
            entity->velY.whole = speed;
            entity->accelY.whole = 0x1800;
            SUCCUBUS_LUNGING(entity) = hit;
            SUCCUBUS_DRAIN_COUNT(entity) = hit;
            SUCCUBUS_DRAIN_TIMER(entity) = hit;
        }
        break;
    case 10:
    {
        u8 t; register u32 z_ asm("r0"); /* FAKEMATCH: r0 live across the test */
        entity->flipX = Entity_IsTargetRight(entity);
        gEwramData->playerState.enemyTarget->flipX = entity->flipX;
        t = entity->timer;
        if (({ asm("" : "=r"(z_)); /* FAKEMATCH: r0 live across the test */ t; }))
        {
            struct EwramData_EntityData *player;
            s32 y, ex, ey; register s32 n asm("r8"); /* FAKEMATCH: divisor in r8 */
            {
                u8 b_ = *(ENTITY_RENDER_FLAGS_ADDRESS(entity)); u32 k_;
                asm("" :: "r"(z_)); /* FAKEMATCH: r0 live across the test */
                k_ = 0x40;
                k_ &= b_;
                if (k_)
                    dx = -0xC0000;
                else
                    dx = 0xC0000;
            }
            player = gEwramData->playerState.player;
            dx = player->posX.whole + dx - (ex = entity->posX.whole);
            y = player->posY.whole; y -= (ey = entity->posY.whole);
            n = *(volatile u8 *)&entity->timer;
            dx /= n;
            y /= n;
            entity->posX.whole = ex + dx;
            entity->posY.whole = ey + y;
            entity->timer = t - 1;
        }
        v = ({ s32 t = SUCCUBUS_DRAIN_TIMER(entity) + 1; SUCCUBUS_DRAIN_TIMER(entity) = t; (s8)t; });
        if (v > 0x2C)
        {
            SUCCUBUS_DRAIN_TIMER(entity) = 0;
            if (({ s32 t = SUCCUBUS_DRAIN_COUNT(entity) + 1; SUCCUBUS_DRAIN_COUNT(entity) = t; (s8)t; }) > 4)
                goto check;
            {
                struct PlayerContactDamage effect;
                effect.attack = entity->attack;
                effect.attributes = 1;
                effect.unk_4 = 0;
                effect.reaction = 0;
                effect.sourceX = entity->posX.whole;
                effect.sourceY = entity->posY.whole;
                Player_ApplyContactDamage(&effect);
            }
            PlaySong(0x7B);
            {
                register u32 z0_ asm("r0"); register u32 z2_ asm("r2"); u8 b_; register u32 k_ asm("r0"); /* FAKEMATCH: and result in r0 */
                asm("" : "=r"(z0_), "=r"(z2_)); /* FAKEMATCH: r0/r2 live across the flag load (reload rotation) */
                b_ = *(ENTITY_RENDER_FLAGS_ADDRESS(entity));
                asm("" :: "r"(z0_), "r"(z2_)); /* FAKEMATCH: r0/r2 live across the flag load (reload rotation) */
                k_ = 0x40;
                k_ &= b_;
                if (k_)
                    dx = 0x10000;
                else
                    dx = -0x10000;
            }
            entity->posX.whole += dx;
        }
        else if (v == 10)
        {
            if (entity->flipX)
                dx = -0x10000;
            else
                dx = 0x10000;
            entity->posX.whole += dx;
        }
    check:
        if ((gEwramData->inputData.playerNewInput & 0x3FF) && *(s8 *)&SUCCUBUS_ESCAPE_PRESSES(entity) <= 0x63)
            SUCCUBUS_ESCAPE_PRESSES(entity)++;
        if (Player_StartScriptedAnimation(0xE, 0, 0, 0) || *(s8 *)&SUCCUBUS_DRAIN_COUNT(entity) > 4 || *(s8 *)&SUCCUBUS_ESCAPE_PRESSES(entity) > 0x1D)
        {
            Anim_StartIndexed(entity, 3, 0, 0);
            {
                register u32 z0_ asm("r0"); register u32 z2_ asm("r2"); u8 b_; register u32 k_ asm("r0"); /* FAKEMATCH: and result in r0 */
                asm("" : "=r"(z0_), "=r"(z2_)); /* FAKEMATCH: r0/r2 live across the flag load (reload rotation) */
                b_ = *(ENTITY_RENDER_FLAGS_ADDRESS(entity));
                asm("" :: "r"(z0_), "r"(z2_)); /* FAKEMATCH: r0/r2 live across the flag load (reload rotation) */
                k_ = 0x40;
                k_ &= b_;
                if (k_)
                    entity->velX.whole = -0x28000;
                else
                    entity->velX.whole = 0x28000;
            }
            entity->accelX.whole = 0;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x1800;
            entity->subState++;
            entity->timer = 0;
            entity->unk_4FC.part8.unk_4FD = 1;
            SUCCUBUS_LUNGING(entity) = 0;
            SUCCUBUS_GRABBED(entity) = 0;
            SUCCUBUS_HOLDING(entity) = 0;
            SUCCUBUS_ESCAPE_PRESSES(entity) = 0;
        }
        break;
    }
    case 11:
        EntityDampVelocityX(entity, 0x18);
        speed2 = 0x10000;
        Entity_MoveWithCollision(entity, speed2, -0x200000, 0x40000, 8);
        Entity_ClampXToRoom(entity, 4);
        if (entity->timer++ > 0x27)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->state = 0;
            entity->subState = 2;
            entity->timer = 0;
            entity->velY.whole = speed2;
            entity->accelY.whole = 0x1800;
            entity->unk_4FC.part8.unk_4FD = 0;
        }
        break;
    }
}
