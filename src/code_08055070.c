/* Animation-driven completion and request flags for the object 0x1C effect. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
struct EffectOwnerState { u8 pad[0x24]; u16 flags; u8 pad26[10]; u8 done, request; };
/**
 * @brief Wait for owner flag 2, play the next animation, request the owner at
 * animation step 2 and delete itself when done.
 */
void SavePoint_UpdateAnimPart(struct EwramData_EntityData *entity)
{
    struct EffectOwnerState *owner = (struct EffectOwnerState *)entity->ext.savePointPart.owner;
    switch (entity->state)
    {
    case 0:
        if (owner->flags & 2) entity->state = 1;
        break;
    case 1:
        switch (entity->animScriptIndex)
        {
        case 0: Anim_StartIndexed(entity, 1, 0, 1); break;
        case 4: Anim_StartIndexed(entity, 5, 0, 1); break;
        }
        entity->state++;
        break;
    case 2:
        switch (entity->animScriptIndex)
        {
        case 2:
        case 7:
            if (owner->done == 1) entity->deleteRequested = 1;
            break;
        case 1:
        case 5:
            if (entity->animStep == 2) owner->request = 1;
            else if (entity->animDone)
            {
                if (--entity->timer == 0)
                {
                    entity->deleteRequested = 1;
                    owner->done = 1;
                }
            }
            break;
        }
        break;
    }
    Anim_Update(entity);
}
