/* Slime setup, dispatch and shrinking death animation. */
#include "code_08009A0.h"
#include "code_08001194.h"
#include "code_080D73B8.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void Slime_UpdateCrawl(struct EwramData_EntityData *);
extern void Slime_UpdateDetach(struct EwramData_EntityData *);
extern void Slime_SpawnParticles(struct EwramData_EntityData *, s32, s32, s32);
extern void Slime_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Slime_OnHit(struct EwramData_EntityData *);
extern u8 sSlimeGfx[], sSlimePalette[], sSlimeSpriteSheet[], sSlimeWalkProfile[];

#include "code/code_080211F0.h"
extern s32 Slime_UpdateSurfaceSupport(struct EwramData_EntityData *, u16);
extern const u16 sSlimeSurfaceTurnTargets[];

/**
 * @brief Detach state (1): animate departure from wall or ceiling, move
 * under gravity, emit particles on surface contact, then align and
 * recover to crawling.
 */
void Slime_UpdateDetach(struct EwramData_EntityData *entity)
{
    s32 baseSpeed = 0x8000;
    s32 flags;
    switch (entity->subState)
    {
    case 0:
        Anim_StartIndexed(entity, 1, 0, 0);
        entity->subState = 1;
        /* fall through */
    case 1:
        if (!(entity->animFlags & 4)) break;
        Anim_StartIndexed(entity, 2, 0, 1);
        entity->ext.slime.flags |= 4;
        switch (entity->stage)
        {
        case 2:
            entity->ext.slime.speed = 0;
            entity->velY.whole = 0;
            break;
        case 1:
            {
                u32 random = RandomNumberGenerator();
                s32 mask = 0x1FFF;
                mask &= random;
                entity->ext.slime.speed = -(baseSpeed + mask);
            }
            goto randomY;
        case 3:
            {
                u32 random = RandomNumberGenerator();
                s32 mask = 0x1FFF;
                mask &= random;
                entity->ext.slime.speed = baseSpeed + mask;
            }
randomY:
            entity->velY.whole = RandomSigned(0x8000) - 0x10000;
            break;
        }
        PlaySong(0x164);
        entity->accelY.whole = 0x2000;
        entity->subState = 2;
        /* fall through */
    case 2:
        flags = Entity_Walk(entity, entity->ext.slime.speed, sSlimeWalkProfile, 0x22);
        if (flags & 8)
        {
            Slime_SpawnParticles(entity, 3, 0, -0x20000);
            entity->stage = 0;
            goto landed;
        }
        if (flags & 1)
        {
            Slime_SpawnParticles(entity, 3, 0x10000, -0x18000);
            entity->stage = 3;
            entity->subState = 3;
            break;
        }
        if (flags & 2)
        {
            Slime_SpawnParticles(entity, 3, -0x10000, -0x18000);
            entity->stage = 1;
landed:
            entity->subState = 3;
        }
        break;
    case 3:
        PlaySong(0x163);
        entity->ext.slime.angle = sSlimeSurfaceTurnTargets[entity->stage];
        entity->ext.slime.flags &= ~4;
        Slime_UpdateSurfaceSupport(entity, 0x4000);
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->subState = 4;
        /* fall through */
    case 4:
        if (entity->animFlags & 4)
        {
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->state = 0;
            entity->subState = 0;
            entity->ext.slime.turnCooldown = 0;
        }
        break;
    }
}
