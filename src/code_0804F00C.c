/* Object 0x05 (door / gate opening near the player), 0x0804F00C..0x0804F15F. */
#include "code_080D73B8.h"
#include "door_entity.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "structs/main.h"

extern void Door_SetupGraphics(struct EwramData_EntityData *entity);

/**
 * @brief 4F00C | Object 0x05 create: drawing set up by Door_SetupGraphics, frame
 * 0x7D, palette 7, window outside 0x3F / 0x2F; start open (unk_4F8 = 0x30)
 * when the player is already in front of it.
 */
void Object05Create(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player;

    Door_SetupGraphics(entity);
    entity->animFrame = 0x7D;
    entity->palette = 7;
    gDisplayRegisters.winOut_L = 0x3F;
    gDisplayRegisters.winOut_H = 0x2F;
    player = gEwramData->playerState.player;
    if (Entity_IsInRect(entity, (s16)(player->posX.part16.integer - 0x1C), player->posY.part16.integer, 0x38, 0x22))
        entity->unk_4F8.whole = 0x30;
}

/**
 * @brief 4F084 | Object 0x05 update: hidden off screen; opens (unk_4F8 up
 * to 0x30 in steps of 2) while the player is within 0x28 px horizontally,
 * closes otherwise, with sound 0x113 when it starts moving on screen.
 */
void Object05Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player = gEwramData->playerState.player;

    entity->hidden = Entity_IsInRect(entity, -0x20, 0, 0x130, 0xD0) ? 0 : 1;
    if (entity->state != 0)
        return;

    if ((u32)((s16)entity->posX.part16.integer - (s16)player->posX.part16.integer + 0x28) <= 0x4F)
    {
        if (DOOR_OPEN_HEIGHT(entity) == 0 && Entity_IsInRect(entity, -0x10, 0, 0x110, 0xC0))
            PlaySong(0x113);
        DOOR_OPEN_HEIGHT(entity) += 2;
        if (DOOR_OPEN_HEIGHT(entity) > 0x2F)
            DOOR_OPEN_HEIGHT(entity) = 0x30;
    }
    else
    {
        if (DOOR_OPEN_HEIGHT(entity) == 0x30 && Entity_IsInRect(entity, -0x10, 0, 0x110, 0xC0))
            PlaySong(0x113);
        if (DOOR_OPEN_HEIGHT(entity) <= 2)
            DOOR_OPEN_HEIGHT(entity) = 0;
        else
            DOOR_OPEN_HEIGHT(entity) -= 2;
    }
}
