/* Dispatch Gorgon/Catoblepas states and support an overridden single frame. */
#include "code_08009A0.h"
#include "gorgon_entity.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "code/code_080211F0.h"
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void Effect_SpawnRisingParticle(s32, s32, s32, s32);
extern void Gorgon_UpdateWalk(struct EwramData_EntityData *);
extern void Gorgon_UpdateAttack(struct EwramData_EntityData *);
extern void Gorgon_SpawnDeathPieces(struct EwramData_EntityData *);
struct SingleFrameAnimation {
    u16 count;
    u16 format;
    u8 frame;
    u8 duration;
    u16 unk_6;
};
extern struct SingleFrameAnimation sGorgonFrameAnim;
void EnemyGorgonUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct SingleFrameAnimation animation = sGorgonFrameAnim;
    entity->ext.gorgon.flags = (u8)entity->ext.gorgon.flags & 0xFE;
    if (entity->state != 2)
    {
        if (EntityIsOutsideScreen(entity, 4)) goto finished;
        if (Entity_IsInRect(entity, -16, 0, 272, 176)) Player_TrackEntity(entity);
        if (entity->state != 2) Player_QueueEntity(entity, 1);
    }
    if (EntityIsOutsideScreen(entity, 4)) entity->hidden = 1;
    else entity->hidden = 0;
    if (entity->state != 2) Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity))
    {
        entity->velX.whole = 0;
        if (entity->state != 2)
        {
            Hit_CheckWithPlayer(collision, entity);
            goto finished;
        }
    }
    switch (entity->state)
    {
    case 0: Gorgon_UpdateWalk(entity); break;
    case 1: Gorgon_UpdateAttack(entity); break;
    case 2:
        if (entity->stage == 0)
        {
            entity->stage = 1;
            Gorgon_SpawnDeathPieces(entity);
        }
        entity->ext.gorgon.flags |= 1;
        entity->ext.gorgon.forcedFrame = 23;
        if ((entity->timer & 3) == 0)
        {
            s32 x = RandomNumberGenerator() % 0x280000 - 0x140000;
            RandomNumberGenerator();
            Effect_SpawnRisingParticle(entity->posX.whole + x, entity->posY.whole, entity->spriteLayer - 1, 1);
        }
        if ((s8)--entity->timer <= 0) entity->deleteRequested = 1;
        Enemy_UpdateFragmentDeath(entity);
        break;
    case 3:
        Anim_Update(entity);
        goto finished;
    }
    {
        u8 *initial = &entity->animFrame;
        u8 previous = *initial;
        s32 override = entity->ext.gorgon.flags & 1;
        u8 *frame = initial;
        if (override)
        {
            Anim_Start(entity, (struct AnimScript *)&animation, 0, 1);
            animation.frame = entity->ext.gorgon.forcedFrame;
            Anim_Update(entity);
        }
        else Anim_Update(entity);
        if (previous != *frame) Entity_UpdateBoxes(entity);
    }
    entity->ext.gorgon.hit = 0;
    Enemy_CheckHitPlayer(entity);
finished:
    return;
}
