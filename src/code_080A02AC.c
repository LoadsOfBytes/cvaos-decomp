/* Entity that follows the player at a fixed offset (KyomaDemon_UpdateLurk). Needs
 * -fno-cse-follow-jumps (Makefile): the target recomputes ew + 0x13110 after
 * the range check instead of reusing it across the jump. */
#include "code_08009A0.h"
#include "kyoma_demon_entity.h"
#include "player_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
#include "structs/entity_render_flags.h"

extern u8 sUnk_08528340[];

/* Byte 0x58 of an entity (unk_53C) viewed as plain u8 bitfields. */
struct EntityFlags58
{
    u8 low:6;
    u8 flipX:1;
    u8 flipY:1;
};

/**
 * @brief Lurk state (state 0): hide behind the scenery and wait (random timer in
 * unk_4FE), then appear near the target and start the attack (state 1).
 */
void KyomaDemon_UpdateLurk(struct EwramData_EntityData *entity)
{
    struct EwramData *ew = gEwramData;
    struct BgLayerState *bg = &ew->bgInfo[1];
    s32 px;
    s32 x;
    s32 one;
    s16 *ppx;
    u8 state = entity->subState;

    switch (state)
    {
    case 0:
    {
        s32 playerFlags = ((u8 *)ew->playerState.player)[0x58] >> 6;
        struct EntityFlags58 *flags;

        one = 1;
        flags = (struct EntityFlags58 *)(ENTITY_RENDER_FLAGS_ADDRESS(entity));
        flags->flipX = ~playerFlags & one;
    }
        entity->posX.whole = ew->playerState.player->posX.whole + 0x100000;
        entity->posY.whole = ew->playerState.player->posY.whole;
        ppx = &entity->posX.part16.integer;
        px = *ppx + (s16)bg->xPos.part16.integer;
        x = (s16)KYOMA_DEMON_HOME_X(entity);
        if (px < x - 0x20 || px > x + 0x20)
        {
            entity->hidden = 1;
            break;
        }
        entity->hidden = 0;
        if (PLAYER_ACTION_FLAGS(ew->playerState.player) & 2)
        {
            Anim_Start(entity, (struct AnimScript *)(sUnk_08528340 + 0x24), 0, 0);
            entity->subState = one;
            break;
        }
        if (KYOMA_DEMON_LURK_TIMER(entity) <= 0)
        {
            KYOMA_DEMON_LURK_TIMER(entity) = RandomNumberGenerator() % 64 + 64;
            px = *ppx + (s16)bg->xPos.part16.integer;
            x = (s16)KYOMA_DEMON_HOME_X(entity);
            if (px > x - 10 && px < x + 10)
            {
                entity->state = one;
                entity->subState = 0;
                break;
            }
        }
        else
        {
            KYOMA_DEMON_LURK_TIMER(entity)--;
        }
        if (entity->posX.whole + bg->xPos.whole == KYOMA_DEMON_ROOM_X(entity)
            && entity->posY.whole + bg->yPos.whole == KYOMA_DEMON_ROOM_Y(entity))
        {
            if (entity->animScriptIndex)
                Anim_StartIndexed(entity, 0, 0, 1);
        }
        else if (entity->animScriptIndex != 1)
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            Anim_Update(entity);
            entity->animTimer = 0x7F;
            Anim_Update(entity);
        }
        break;
    case 1:
        entity->flipX = state & ~(((u8 *)ew->playerState.player)[0x58] >> 6);
        entity->posX.whole = ew->playerState.player->posX.whole + 0x100000;
        entity->posY.whole = ew->playerState.player->posY.whole;
        px = (s16)entity->posX.part16.integer + (s16)bg->xPos.part16.integer;
        x = (s16)KYOMA_DEMON_HOME_X(entity);
        if (px < x - 0x20 || px > x + 0x20)
            entity->hidden = 1;
        else
            entity->hidden = 0;
        if (!(PLAYER_ACTION_FLAGS(gEwramData->playerState.player) & 2))
        {
            Anim_StartIndexed(entity, 1, 0, 1);
            entity->subState = 0;
        }
        break;
    }
}
