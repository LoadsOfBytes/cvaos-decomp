/* PlayerAfterimage_Update. */
#include "code/code_08039340.h"
#include "structs/ewram.h"

/**
 * @brief 2EF28 | Update via Sprite_DrawEntity; every subState + 1 frames (counter
 * timer) run the player's draw function with the player moved to the
 * entity's position (afterimage), then restore the player's position.
 */
void PlayerAfterimage_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *player = gEwramData->playerState.player;
    u16 x = player->posX.part16.integer;
    u16 y = player->posY.part16.integer;

    Sprite_DrawEntity(entity);
    if (++entity->timer > entity->subState)
        entity->timer = 0;
    if (entity->timer == 0)
    {
        struct PlayerState *p = &gEwramData->playerState;
        p->player->posX.part16.integer = entity->posX.part16.integer;
        p->player->posY.part16.integer = entity->posY.part16.integer;
        ((void (*)(struct EwramData_EntityData *))p->player->drawFunc)(p->player);
        gEwramData->playerState.player->posX.part16.integer = x;
        gEwramData->playerState.player->posY.part16.integer = y;
    }
}
