/* Effect_FollowParentHitBox. */
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"

extern u8 sCommonObjectGfx[];
extern u8 sCommonObjectSpriteSheet[];

/**
 * @brief C799C | Follow the parent (entity unk_4F5) at the offset returned
 * by HitBox_GetBoxes; state 0 first takes the loaded graphics sCommonObjectGfx
 * (palette 7, animation 0).
 */
void Effect_FollowParentHitBox(struct EwramData_EntityData *entity)
{
    struct HitBoxRecord attackBox;
    struct HitBoxRecord bodyBox;
    struct EwramData_EntityData *parent = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];

    HitBox_GetBoxes(&attackBox, &bodyBox, parent);
    switch (entity->state)
    {
        case 0:
        {
            s32 gfx = ObjGfx_Find(sCommonObjectGfx);

            Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 7);
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            entity->state++;
        }
        case 1:
            entity->posX.part16.integer = parent->posX.part16.integer + attackBox.x;
            entity->posY.part16.integer = parent->posY.part16.integer + attackBox.y;
            break;
    }
}
