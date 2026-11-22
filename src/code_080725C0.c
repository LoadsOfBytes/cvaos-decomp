/* Advance a debris sprite's motion and its three successive frames. */
#include "code_08009A0.h"
#include "code/code_08060B98.h"
/**
 * @brief Mud piece update: fall and step the frame at 6 and 12 frames; delete
 * after 18 frames.
 */
void MudmanPiece_Update(struct EwramData_EntityData *entity)
{
    entity->timer++;
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->timer == 6) entity->animFrame++;
    if (entity->timer == 12) entity->animFrame++;
    if (entity->timer > 17) EntityDelete(entity);
}
