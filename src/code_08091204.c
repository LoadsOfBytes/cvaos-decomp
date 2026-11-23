#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern void DevilPart_Attach(struct EwramData_EntityData *, s32, s32, s32, s32);
extern void Devil_UpdateAllParts(struct EwramData_EntityData *);
extern void Devil_SaveParts(struct EwramData_EntityData *);
extern void Devil_RestoreParts(struct EwramData_EntityData *);

/**
 * @brief 91204 | Swinging body-part controller: every 22 frames (timer)
 * flips the swing direction (unk_504 = 1 / 2) and re-launches the six parts
 * via DevilPart_Attach (index, parent, angular speed +-0xA00 / +-0x800, radius
 * 0xA0000 / 0xD0000 / 0xB0000); states 1 and 3 pause until the counter
 * drops to 2. Devil_UpdateAllParts updates the parts every frame.
 */
void Devil_UpdateHover(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        entity->subState++;
        Anim_StartIndexed(entity, entity->animScriptIndex + 1, 0, 1);
        entity->timer = 0x16;
        entity->ext.joint.pose = 1;
        Devil_SaveParts(entity);
        DevilPart_Attach(entity, 1, 0, -0xA00, 0xA0000);
        DevilPart_Attach(entity, 2, 0, -0xA00, 0xD0000);
        DevilPart_Attach(entity, 3, 2, -0x800, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0xA00, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0xA00, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x800, 0xB0000);
        break;
    case 2:
        Devil_UpdateAllParts(entity);
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 0x16;
        entity->subState++;
        entity->ext.joint.pose = 2;
        DevilPart_Attach(entity, 1, 0, 0xA00, 0xA0000);
        DevilPart_Attach(entity, 2, 0, 0xA00, 0xD0000);
        DevilPart_Attach(entity, 3, 2, 0x800, 0xB0000);
        DevilPart_Attach(entity, 5, 4, -0xA00, 0xA0000);
        DevilPart_Attach(entity, 6, 4, -0xA00, 0xD0000);
        DevilPart_Attach(entity, 7, 6, -0x800, 0xB0000);
        break;
    case 1:
    case 3:
        Devil_UpdateAllParts(entity);
        if ((s8)--entity->timer <= 2)
        {
            entity->timer = 8;
            entity->subState++;
        }
        break;
    case 4:
        Devil_UpdateAllParts(entity);
        if ((s8)--entity->timer > 0)
            break;
        entity->timer = 0x16;
        entity->subState = 1;
        entity->ext.joint.pose = 1;
        Devil_RestoreParts(entity);
        DevilPart_Attach(entity, 1, 0, -0xA00, 0xA0000);
        DevilPart_Attach(entity, 2, 0, -0xA00, 0xD0000);
        DevilPart_Attach(entity, 3, 2, -0x800, 0xB0000);
        DevilPart_Attach(entity, 5, 4, 0xA00, 0xA0000);
        DevilPart_Attach(entity, 6, 4, 0xA00, 0xD0000);
        DevilPart_Attach(entity, 7, 6, 0x800, 0xB0000);
        break;
    }
}
