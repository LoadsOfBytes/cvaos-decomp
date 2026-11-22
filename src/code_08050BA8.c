/* Draw the entity one pixel lower, then its alternate frame every other pair of frames. */
#include "code/code_08039340.h"
#include "structs/ewram.h"
struct AlternateFrameInfo { u8 pad[5]; u8 frame; u8 rest[6]; };
extern struct AlternateFrameInfo sObject0EVariants[];
/**
 * @brief Unless hidden, animate and draw one pixel lower; every other two frames
 * also draw the variant's overlay frame when it has one.
 */
void Object0E_Draw(struct EwramData_EntityData *entity)
{
    struct AlternateFrameInfo *info;
    if (entity->hidden) return;
    info = &sObject0EVariants[entity->unk_514.part16.spawnParam0];
    Anim_Update(entity);
    entity->posY.part16.integer++;
    Sprite_Draw(entity);
    entity->posY.part16.integer--;
    if (info->frame != 0 && (gEwramData->frameCounter & 2))
    {
        u8 frame = entity->animFrame;
        entity->animFrame = info->frame;
        Sprite_Draw(entity);
        entity->animFrame = frame;
    }
}
