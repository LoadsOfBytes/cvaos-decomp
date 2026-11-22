/* Dispatch Giant Ghost states and emit particles during its death fade. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *);
extern s32 Entity_GetVisibilityPattern(s32);
extern void GiantGhost_UpdateMove(struct EwramData_EntityData *);
extern void GiantGhost_UpdateAttack(struct EwramData_EntityData *);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *);
extern void GiantGhost_SpawnFragment(s32, s32, s32, s32, s32, s32);
void EnemyGiantGhostUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    if (Enemy_PreUpdate(entity)) goto finished;
    switch (entity->state)
    {
    case 0: GiantGhost_UpdateMove(entity); break;
    case 1: GiantGhost_UpdateAttack(entity); break;
    case 2:
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if ((s8)--entity->timer <= 0) entity->deleteRequested = 1;
        if (entity->timer > 40 && (entity->timer & 7) == 1)
        {
            s32 x = (RandomNumberGenerator() & 0x1FFFFF) - 0x100000;
            s32 y = -(RandomNumberGenerator() & 0x1FFFFF);
            s32 vx = (RandomNumberGenerator() & 0x7FFF) - 0x4000;
            s32 type = RandomNumberGenerator() & 1;
            GiantGhost_SpawnFragment(entity->posX.whole + x, entity->posY.whole + y, vx, 0, entity->spriteLayer - 1, type);
        }
        break;
    case 3:
        Anim_Update(entity);
        goto finished;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
finished:
    return;
}
