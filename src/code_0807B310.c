/* Zombie debris and temporary effects. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void ZombiePiece_Update(struct EwramData_EntityData *);
extern u8 sZombieGfx[], sZombieSpriteSheet[];
extern void ZombieDebris_Update(struct EwramData_EntityData *);
extern void ZombieTemporarySprite_Update(struct EwramData_EntityData *);
/**
 * @brief Apply vertical acceleration and movement; delete when the signed byte timer expires.
 */
void ZombieDebris_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateY(entity, 0x80000);
    EntityMoveY(entity);
    if ((s8)entity->timer-- <= 0) EntityDelete(entity);
}
