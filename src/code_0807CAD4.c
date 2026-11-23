/* Activate a timed projectile and retain it only while its parent is alive. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Effect_SpawnFallingPiece(s32, s32, s32, s32, s32, s32);
void AluraUneThorn_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    struct HitBoxRecord box = {-2, -54, 4, 54};
    struct EwramData_EntityData *parent = ENTITY_OWNER(entity);
    switch (entity->state)
    {
    case 0:
        if ((s8)entity->timer-- <= 0)
        {
            Anim_StartIndexed(entity, 12, 0, 0);
            entity->hidden = 0;
            entity->state++;
        }
        break;
    case 1:
        if (entity->animDone)
        {
            if (parent && parent->updateFunc && parent->enemyId == 0x4D && parent->state != 2)
            {
                entity->state++;
                entity->timer = 0;
                Anim_StartIndexed(entity, 7, 0, 0);
            }
            else
            {
                entity->deleteRequested = 1;
                goto finished;
            }
        }
        break;
    case 2:
        if (++entity->timer == 8)
        {
            s32 vx = (RandomNumberGenerator() & 0x1FFFF) - 0x10000;
            s32 vy = -0x20000 - (RandomNumberGenerator() & 0x1FFFF);
            s32 type = RandomNumberGenerator() & 1;
            Effect_SpawnFallingPiece(entity->posX.whole, entity->posY.whole, vx, vy, entity->spriteLayer - 1, type);
        }
        if (entity->animDone)
        {
            entity->state++;
            Anim_StartIndexed(entity, 8, 0, 1);
            entity->timer = 0;
            HitBox_SetAttack(entity, &box);
            entity->hit.flags &= ~2;
            entity->hit.flags |= 4;
        }
        break;
    case 3:
        Hit_CheckWithPlayer(collision, entity);
        if (entity->timer++ >= entity->ext.aluraUneThorn.time)
        {
            entity->state++;
            Anim_StartIndexed(entity, 13, 0, 0);
        }
        if (!parent || !parent->updateFunc || parent->enemyId != 0x4D || parent->state == 2)
        {
            Anim_StartIndexed(entity, 13, 0, 0);
            entity->state++;
            goto finished;
        }
        break;
    case 4:
        if (entity->animDone)
        {
            entity->deleteRequested = 1;
            goto finished;
        }
        break;
    }
    Anim_Update(entity);
finished:
    return;
}
