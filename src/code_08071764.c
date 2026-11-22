/* Move the projectile or emit its two fragments before deleting it. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern void EntityMoveX(struct EwramData_EntityData *);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);
extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
/**
 * @brief Shot update: fly until off screen (state 0); state 2 bursts into two
 * random effects and deletes the shot.
 */
void MermanShot_Update(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    switch (entity->state)
    {
    case 0:
        EntityMoveX(entity);
        if (!EntityIsOutsideScreen(entity, 1)) goto animate;
        goto remove;
    case 1: break;
    case 2:
        Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0xA0000, 0x40000, entity->spriteLayer);
        Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0xA0000, 0x40000, entity->spriteLayer);
        goto remove;
    }
    goto animate;
remove:
    EntityDelete(entity);
    goto finished;
animate:
    Hit_CheckWithPlayer(collision, entity);
    Anim_Update(entity);
finished:
    return;
}

