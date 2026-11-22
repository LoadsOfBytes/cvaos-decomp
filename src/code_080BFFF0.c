/* Kicker Skeleton leap, kick, landing and recovery. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"

extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern void Afterimage_Create(struct EwramData_EntityData *, s32, s32, s32);
extern u8 sKickerSkeletonWalkProfile[];
extern s32 __divsi3(s32, s32);

/**
 * @brief Attack state (1): prepare, leap and dive-kick with afterimages,
 * then land and brake before returning to idle. Kick flag 1 controls
 * the enhanced contact damage. Preserve the halfword animation-event test.
 */
void KickerSkeleton_UpdateKick(struct EwramData_EntityData *entity)
{
    s32 flags;
    s32 x;
    s32 y;
    s32 timer;
    struct EwramData_EntityData *particle;
    switch (entity->subState)
    {
    case 0:
        Entity_FaceTarget(entity);
        entity->subState = 1;
        /* fall through */
    case 1:
        Anim_StartIndexed(entity, 3, 0, 0);
        entity->subState = 2;
        break;
    case 2:
        if (!(entity->animFlags & 4)) break;
        Anim_StartIndexed(entity, 4, 0, 0);
        entity->unk_4FC.whole = 0x8000;
        entity->velY.whole = -0x30000;
        entity->accelY.whole = 0x1000;
        entity->subState = 3;
        break;
    case 3:
        flags = Entity_Walk(entity, entity->unk_4FC.whole, sKickerSkeletonWalkProfile, 0);
        if (flags & 4) entity->subState = 7;
        else if (entity->velY.whole >= -0x18000)
        {
            Entity_FaceTarget(entity);
            Anim_StartIndexed(entity, 6, 0, 0);
            entity->subState = 4;
        }
        break;
    case 4:
        Entity_Walk(entity, entity->unk_4FC.whole, sKickerSkeletonWalkProfile, 0);
        /* The ROM tests the frame and its adjacent byte as one halfword. */
        if (*(u16 *)&entity->animStep != 5) break;
        Anim_StartIndexed(entity, 5, 0, 0);
        Afterimage_Create(entity, 0x30, 0xC, 0x11);
        Afterimage_Create(entity, 0x30, 6, 0x10);
        entity->unk_4FC.whole = 0x28000;
        entity->velY.whole = 0x30000;
        entity->unk_4F8.whole |= 1;
        PlaySong(0x87);
        entity->subState = 5;
        break;
    case 5:
        flags = Entity_Walk(entity, entity->unk_4FC.whole, sKickerSkeletonWalkProfile, 0);
        if (flags & 8)
        {
            Anim_StartIndexed(entity, 3, 0, 0);
            entity->unk_4F8.whole &= ~1;
            PlaySong(0x127);
            entity->subState = 6;
        }
        break;
    case 6:
        flags = Entity_Walk(entity, entity->unk_4FC.whole, sKickerSkeletonWalkProfile, 0);
        {
            s32 speed = entity->unk_4FC.whole;
            s32 value = 31;
            value *= speed;
            entity->unk_4FC.whole = __divsi3(value, 32);
        }
        if ((s32)entity->unk_4FC.whole <= 0x7FFF) goto finished;
        if (!(flags & 8)) break;
        if (flags & 3) entity->unk_4FC.whole = 0;
        if ((s32)entity->unk_4FC.whole <= 0xBFFF) break;
        timer = entity->timer + 1;
        entity->timer = timer;
        if (timer & 3) break;
        x = entity->posX.whole + (RandomSigned(0x10) << 16);
        y = entity->posY.whole + (RandomSigned(4) << 16) + 0xC0000;
        particle = Effect_SpawnShared(x, y, (RandomNumberGenerator() & 1) + 4, 6);
        if (particle == NULL) break;
        particle->drawSkip = 1;
        particle->velX.whole = 0x8000;
        if (entity->flipX) particle->velX.whole = -particle->velX.whole;
        particle->accelY.whole = -(RandomNumberGenerator() & 0x3FF);
        break;
    case 7:
        flags = Entity_Walk(entity, entity->unk_4FC.whole, sKickerSkeletonWalkProfile, 0);
        if (flags & 8)
        {
finished:
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->state = 0;
            entity->stage = 0;
            entity->subState = 0;
        }
        break;
    }
}
