/* Scene start: place the player and the three helper entities at fixed room
 * positions (relative to the BG1 camera) and give each helper its animation. */
#include "code/code_08039340.h"
#include "event_scene03.h"
#include "structs/ewram.h"

extern void Player_StartScriptedAnimation(s32 anim, s32 flags, s32 velX, s32 velY);
void EventScene03MainPart_Update(struct EwramData_EntityData *entity);

#define PLAYER (gEwramData->playerState.player)

/* The camera X reads use block-local u16 copies; the 0x16F position is
 * derived from the 0x119 literal register by CSE. The result of the last
 * animation call stays live in r0, so it is returned. */
s32 EventScene03_Start(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg1, *bg2, *bg3;
    s32 result;

    Player_StartScriptedAnimation(0x51, 0, 0, 0);
    PLAYER->posX.part16.integer = 0xA8 - (bg1 = &gEwramData->bgInfo[1])->xPos.part16.integer;
    PLAYER->posY.part16.integer = 0x2C0 - bg1->yPos.part16.integer;
    PLAYER->velX.whole = 0;
    PLAYER->mosaic = 1;

    EVENT03_ACTOR0(entity)->posX.part16.integer = PLAYER->posX.part16.integer - 0x18;
    EVENT03_ACTOR0(entity)->posY.part16.integer = PLAYER->posY.part16.integer;
    if (EVENT03_ACTOR0(entity)->animScriptIndex != 4)
        Anim_StartIndexed(EVENT03_ACTOR0(entity), 4, 0, 0);
    EVENT03_ACTOR0(entity)->mosaic = 1;
    ENTITY_PARENT_INDEX(EVENT03_ACTOR0(entity)) = ENTITY_INDEX(entity);

    {
        struct EwramData_EntityData *child = EVENT03_ACTOR1(entity);
        u16 camX = (bg2 = &gEwramData->bgInfo[1])->xPos.part16.integer;

        child->posX.part16.integer = 0x119 - camX;
    }
    EVENT03_ACTOR1(entity)->posY.part16.integer = 0x2C0 - bg2->yPos.part16.integer;
    Anim_StartIndexed(EVENT03_ACTOR1(entity), 1, 0, 1);
    EVENT03_ACTOR1(entity)->velX.whole = -0x6000;
    ENTITY_PARENT_INDEX(EVENT03_ACTOR1(entity)) = ENTITY_INDEX(entity);

    {
        struct EwramData_EntityData *child = EVENT03_ACTOR8(entity);
        u16 camX = (bg3 = &gEwramData->bgInfo[1])->xPos.part16.integer;

        child->posX.part16.integer = 0x16F - camX;
    }
    EVENT03_ACTOR8(entity)->posY.part16.integer = 0x268 - bg3->yPos.part16.integer;
    result = Anim_StartIndexed(EVENT03_ACTOR8(entity), 4, 0, 1);
    ENTITY_PARENT_INDEX(EVENT03_ACTOR8(entity)) = ENTITY_INDEX(entity);
    EVENT03_ACTOR8(entity)->updateFunc = (u32 *)EventScene03MainPart_Update;
    return result;
}
