/* Zombie debris and temporary effects. */
#include "code_08009A0.h"
#include "zombie_entity.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void ZombiePiece_Update(struct EwramData_EntityData *);
extern u8 sZombieGfx[], sZombieSpriteSheet[];
extern void ZombieDebris_Update(struct EwramData_EntityData *);
extern void ZombieTemporarySprite_Update(struct EwramData_EntityData *);
/**
 * @brief Delete the temporary sprite when its 32-bit lifetime counter expires.
 */
void ZombieTemporarySprite_Update(struct EwramData_EntityData *entity)
{
    if (ZOMBIE_SPRITE_LIFETIME(entity)-- <= 0) EntityDelete(entity);
}
