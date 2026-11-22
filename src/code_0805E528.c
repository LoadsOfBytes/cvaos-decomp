/* EventScene03_SpawnRisingPart. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Entity_CreateFromObjectTable(s32 kind);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
void EventScene03RisingPart_Update(struct EwramData_EntityData *entity);

/**
 * @brief 5E528 | Spawn an effect entity (Entity_CreateFromObjectTable(10), update
 * EventScene03RisingPart_Update) at (x, y) with parent's unk_4F4 in unk_4F5 and flags in
 * unk_4F7; flag 4 moves it 8 px down (16 steps of 0x8000) into layer
 * 0x17. Animation 0, facing the player.
 */
struct EwramData_EntityData *EventScene03_SpawnRisingPart(struct EwramData_EntityData *parent, s32 x, s32 y, s32 flags)
{
    struct EwramData_EntityData *entity = Entity_CreateFromObjectTable(10);
    s32 flip;

    entity->updateFunc = (u32 *)EventScene03RisingPart_Update;
    entity->posX.part16.integer = x;
    entity->posY.part16.integer = y;
    Entity_SnapToGround(entity);
    entity->unk_4F4.part8.unk_4F5 = parent->unk_4F4.part8.unk_4F4;
    entity->unk_4F4.part8.unk_4F7 = flags;
    entity->unk_518.part8.enemyId = 0xFF;
    Anim_StartIndexed(entity, 0, 0, 1);
    if (entity->unk_4F4.part8.unk_4F7 & 4)
    {
        s32 i;

        for (i = 16; i != 0; i--)
            entity->posY.whole += 0x8000;
        entity->spriteLayer = 0x17;
    }
    flip = 0;
    if (entity->posX.whole < gEwramData->playerState.player->posX.whole)
        flip = 1;
    entity->flipX = flip;
    return entity;
}
