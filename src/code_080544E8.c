/* SavePoint_DrawPromptMarkers (0x080544E8..0x08054573). */
#include "code/code_08039340.h"
#include "save_point_entity.h"
#include "structs/ewram.h"

/**
 * @brief Draw the prompt sprite parts at fixed screen positions; frames
 * 0x16/0x17 and 0x19/0x18 follow the yes/no choice unk_4FA of the save
 * point in slot unk_4F5.
 */
void SavePoint_DrawPromptMarkers(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *prompt = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];

    if (prompt != NULL)
    {
        entity->posX.part16.integer = 0x48;
        entity->posY.part16.integer = 0x26;
        entity->animFrame = 0x15;
        Sprite_Draw(entity);
        entity->posX.part16.integer = 0x39;
        entity->posY.part16.integer = 0x3E;
        if (SAVE_POINT_YES_SELECTED(prompt))
            entity->animFrame = 0x16;
        else
            entity->animFrame = 0x17;
        Sprite_Draw(entity);
        entity->posX.part16.integer = 0x99;
        if (SAVE_POINT_YES_SELECTED(prompt))
            entity->animFrame = 0x19;
        else
            entity->animFrame = 0x18;
        Sprite_Draw(entity);
    }
}
