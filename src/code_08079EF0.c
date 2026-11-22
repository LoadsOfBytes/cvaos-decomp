/* Arachne_UpdateAttack: breath attack (hover states 0 / 10, wind-up state 1, state 2 traces the breath projectile until it hits a wall). */
#include "code_08009A0.h"
#include "arachne_entity.h"
#include "entity_hit.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"

extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);
extern s32 Sound_PlayTracked(s32 song);
extern s32 Sound_PlayTrackedIfAbsent(s32 song);
extern s32 Sound_StopTracked(s32 song);
extern void EntitySetVelocityFromAngle(struct EwramData_EntityData *, s32, s32);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 Entity_MoveWithCollisionDefault(struct EwramData_EntityData *, s32, s32);
extern void Arachne_SpawnEffect(struct EwramData_EntityData *owner, s32 x, s32 y, s32 animation, s32 lifetime);
extern void ArachneEffect_Update(struct EwramData_EntityData *);
extern void ArachneBeam_DamagePlayerOnContact(struct EwramData_EntityData *);

#define WAIT(e) ((e)->unk_504.part8.unk_506)

#define HOVER(entity)                                                       \
    {                                                                       \
        s32 phase;                                                          \
                                                                            \
        if (ARACHNE_WALK_SPEED(entity) < 0)                                 \
            ARACHNE_WALK_SPEED(entity) += 0x2000;                           \
        else if (ARACHNE_WALK_SPEED(entity) > 0)                            \
            ARACHNE_WALK_SPEED(entity) -= 0x2000;                           \
        phase = ARACHNE_WALK_PHASE(entity) + ARACHNE_WALK_SPEED(entity);    \
        ARACHNE_WALK_PHASE(entity) = phase;                                 \
        if (phase >= 0x240000)                                              \
            ARACHNE_WALK_PHASE(entity) = phase - 0x240000;                  \
        else if (phase < 0)                                                 \
            ARACHNE_WALK_PHASE(entity) = phase + 0x240000;                  \
        EntityDampVelocityX(entity, 8);                                     \
        Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 12);              \
    }

/**
 * @brief Attack state machine (state 1): rear up and sweep a beam traced from the
 * mouth to the first wall each frame (contact ArachneBeam_DamagePlayerOnContact,
 * sparks via Arachne_SpawnEffect) for 60 frames; then back to state 0.
 */
void Arachne_UpdateAttack(struct EwramData_EntityData *entity)
{
    u32 box = 0x0404FEFE;

    switch (entity->subState)
    {
    case 0:
        HOVER(entity)
        if (entity->timer++ > 7)
        {
            entity->timer = 0;
            entity->subState++;
            Anim_StartIndexed(entity, 1, 0, 0);
        }
        break;
    case 1:
        if (*(ENTITY_RENDER_FLAGS_HIGH_ADDRESS(entity)) & 1)
        {
            entity->timer = 0;
            entity->subState++;
            Anim_StartIndexed(entity, 2, 0, 0);
            if (entity->flipX)
                ARACHNE_ATTACK_ANGLE(entity) = 0x2800;
            else
                ARACHNE_ATTACK_ANGLE(entity) = 0x5800;
            Sound_PlayTracked(0x82);
        }
        break;
    case 2:
    {
        s32 x, y;
        struct EwramData_EntityData *p;
        u8 *flags58;

        if (!(({ s32 t = entity->timer + 1; entity->timer = t; t; }) & 3))
            Sound_PlayTrackedIfAbsent(0x82);
        x = entity->posX.whole;
        y = entity->posY.whole - 0x2B0000;
        {
            u8 *f58 = ENTITY_RENDER_FLAGS_ADDRESS(entity);
            s32 f = *f58 & 0x40;

            flags58 = f58;
            if (f)
                x += 0xD0000;
            else
                x -= 0xD0000;
        }
        p = EntityCreateInRange(0x1B, 0x48, (u32 *)ArachneEffect_Update);
        if (p != NULL)
        {
            struct HitResult hit;
            s32 r;
            s32 wall;

            Hit_Enable(p, 8, ArachneBeam_DamagePlayerOnContact, 0);
            HitBox_SetAttack(p, &box);
            *((u8 *)p + 0x72) = (*((u8 *)p + 0x72) & 0xFD) | 4;
            p->posX.whole = x;
            p->posY.whole = y;
            { u8 t = *((u8 *)entity + 0x3C); *((u8 *)p + 0x3C) = t; }
            EntitySetVelocityFromAngle(p, ARACHNE_ATTACK_ANGLE(entity), 0x80000);
            RandomNumberGenerator();
            y = 3;
            r = RandomNumberGenerator() & y;
            wall = 4;
            while (!(Entity_MoveWithCollisionDefault(p, 0, 0) & wall))
            {
                s32 adx;

                if (EntityIsOutsideScreen(p, 0))
                    goto delete;
                x = Target_GetX(entity) - p->posX.whole;
                y = Target_GetY(entity) - p->posY.whole;
                adx = x;
                if (x < 0)
                    adx = -x;
                if (adx <= 0x7FFFF && (u32)(y + 0x1FFFF) <= 0x21FFFE)
                    Hit_CheckWithPlayer(&hit, (u8 *)p);
                if (p->timer <= 3 && r == (p->timer & 3))
                {
                    register u32 m asm("r5"); /* FAKEMATCH: pinned register */

                    x = RandomNumberGenerator();
                    m = 0x1FFFF;
                    x = (x & m) - 0x10000;
                    y = (RandomNumberGenerator() & m) - 0x10000;

                    Arachne_SpawnEffect(entity, p->posX.whole + x, p->posY.whole + y, 7, 1);
                }
                p->timer++;
            }
            if (!(entity->timer & 3))
                Arachne_SpawnEffect(entity, p->posX.whole, p->posY.whole, 5, wall);
        delete:
            EntityDelete(p);
        }
        if (*flags58 & 0x40)
            ARACHNE_ATTACK_ANGLE(entity) -= 0x60;
        else
            ARACHNE_ATTACK_ANGLE(entity) += 0x60;
        if (!(entity->timer & 3))
        {
            s32 rx = RandomNumberGenerator() & 0x3FFFF;
            s32 f;

            y = (RandomNumberGenerator() & 0x3FFFF) - 0x2E0000;
            f = *flags58 & 0x40;
            x = rx - 0xC0000;
            if (f)
                { register s32 k asm("r2") = 0x80000; x = rx + k; } /* FAKEMATCH: pinned register */
            Arachne_SpawnEffect(entity, entity->posX.whole + x, entity->posY.whole + y, 4, 4);
        }
        if (entity->timer > 0x3B)
        {
            entity->state = 0;
            entity->subState = 1;
            entity->timer = 0;
            WAIT(entity) = (RandomNumberGenerator() % 64) + 0x3C;
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = 0x2800;
            Anim_StartIndexed(entity, 0, 0, 1);
            Sound_StopTracked(0x82);
        }
        break;
    }
    case 10:
        HOVER(entity)
        if (entity->timer++ > 7)
        {
            entity->timer = 0;
            entity->subState = 1;
            Anim_StartIndexed(entity, 1, 0, 0);
        }
        break;
    }
}
