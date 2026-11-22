#include "code/code_080211F0.h"
#include "kyoma_demon_entity.h"
#include "code/code_08039340.h"

/* Draw the player inside this effect's horizontal window, with a temporary
 * position, priority and facing, then restore the saved player state. */
static inline struct EwramData_EntityData *CtxV(void) { return *(struct EwramData_EntityData * volatile *)&gEwramData->playerState.player; }
void KyomaDemon_Draw(struct EwramData_EntityData *entity)
{
    struct EwramData *ewram = gEwramData;
    struct BgLayerState *bg = &ewram->bgInfo[1];
    struct EwramData_EntityData *player = ewram->playerState.player;
    if (entity->state == 2)
    {
        s32 center = KYOMA_DEMON_HOME_X(entity);
        s32 x = CtxV()->posX.whole;
        s32 b_ = bg->xPos.whole;
        s32 offset = 0x80000;
        s32 screenX;
        b_ += offset;
        screenX = (x + b_) >> 16;
        if (screenX > (s16)center - 32 && screenX < (s16)center + 32)
        {
            s32 oldX = player->posX.whole;
            s32 oldY = player->posY.whole;
            s32 oldPriority = player->spriteLayer;
            s32 oldFlip = player->flipX;
            u8 old2 = player->drawSkipCounter;
            u8 old3 = player->drawSkip;
            player->posX.whole = x + offset;
            player->posY.whole = CtxV()->posY.whole;
            player->spriteLayer = 29;
            if (gEwramData->playerStatus.currentCharacter == 1)
                player->flipX ^= 1;
            Player_DrawWithOffset(player);
            player->posX.whole = oldX;
            player->posY.whole = oldY;
            player->spriteLayer = oldPriority;
            player->flipX = oldFlip;
            player->drawSkipCounter = old2;
            player->drawSkip = old3;
        }
        Sprite_DrawEntity(entity);
    }
    else Sprite_DrawEntity(entity);
}
