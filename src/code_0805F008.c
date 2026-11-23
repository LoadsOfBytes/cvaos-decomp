/* Related-entity animation and particle helpers, 0x0805F008..0x0805F11F. */
#include "code_080D73B8.h"
#include "event_scene03.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Effect_SpawnAttachedAnimation(struct EwramData_EntityData *, s32, s32, s32, s32);

/**
 * @brief Finish event scene 3: animation 5 on helper unk_4F8, animation 1 on
 * helper unk_4FC and delete helper unk_500.
 */
s32 EventScene03_EndParts(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *actor = EVENT03_ACTOR0(entity);
    s32 result;
    if (actor->animScriptIndex != 5)
        Anim_StartIndexed(actor, 5, 0, 0);
    result = Anim_StartIndexed(EVENT03_ACTOR1(entity), 1, 0, 1);
    EVENT03_ACTOR8(entity)->deleteRequested = 1;
    return result;
}

/* Same ballistic particle update as MandragoraDeathPart_Update, without a parent count. */
void EventScene03Debris_Update(struct EwramData_EntityData *entity)
{
    if (!Entity_IsInRect(entity, -0x80, -0x40, 0x1F0, 0x120)
        || (u8)--entity->timer == 0xFF || entity->posY.part16.integer > 0xAA)
        entity->deleteRequested = 1;
    else
    {
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += 0x3000;
    }
}

/**
 * @brief Delete the soul orb, show the absorb animation on the player and play
 * sound 0xBC.
 */
void EventScene03SoulOrb_OnPlayerContact(struct EwramData_EntityData *entity)
{
    entity->deleteRequested = 1;
    Effect_SpawnAttachedAnimation(gEwramData->playerState.player, 0xC, 6, 0, -0x100000);
    PlaySong(0xBC);
}

/**
 * @brief Delete the orb, show the absorb animation on its target and play sound
 * 0xBC.
 */
void EventScene03TargetOrb_OnContact(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *anchor = EVENT03_ORB_TARGET(entity);
    entity->deleteRequested = 1;
    Effect_SpawnAttachedAnimation(anchor, 0xC, 6, 0, -0x100000);
    PlaySong(0xBC);
}
