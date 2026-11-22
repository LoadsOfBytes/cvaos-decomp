/* EventScene03_SpawnFlyingPart. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Entity_CreateFromObjectTable(s32 kind);
extern u32 RandomNumberGenerator(void);
void EventScene03FlyingPart_Update(struct EwramData_EntityData *entity);

/**
 * @brief 5E260 | Spawn an effect entity (Entity_CreateFromObjectTable(8), update
 * EventScene03FlyingPart_Update) at (x, y) with parent's unk_4F4 in unk_4F5, a random phase
 * in unk_50A, animation 4, facing the player.
 */
struct EwramData_EntityData *EventScene03_SpawnFlyingPart(struct EwramData_EntityData *parent, s32 x, s32 y)
{
    struct EwramData_EntityData *entity = Entity_CreateFromObjectTable(8);
    s32 flip;

    entity->updateFunc = (u32 *)EventScene03FlyingPart_Update;
    entity->posX.part16.integer = x;
    entity->posY.part16.integer = y;
    entity->unk_4F4.part8.unk_4F5 = parent->unk_4F4.part8.unk_4F4;
    entity->unk_508.part16.unk_50A = (RandomNumberGenerator() % 128) << 9;
    Anim_StartIndexed(entity, 4, 0, 1);
    flip = 0;
    if (entity->posX.whole < gEwramData->playerState.player->posX.whole)
        flip = 1;
    entity->flipX = flip;
    return entity;
}
