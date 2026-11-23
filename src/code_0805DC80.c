#include "code/code_08039340.h"
#include "event_scene03.h"
#include "structs/ewram.h"

extern struct EwramData_EntityData *Entity_CreateFromObjectTable(s32);
extern void EventScene03UnusedPart_Update(struct EwramData_EntityData *);
extern void EventScene03AttachedEffect_Update(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);

/* Spawn an entity and an attached animated part in event slot group 9. */
struct EwramData_EntityData *EventScene03_SpawnUnusedPart(struct EwramData_EntityData *owner, s32 x, s32 y, s32 flags)
{
    struct EwramData_EntityData *entity = Entity_CreateFromObjectTable(9);
    struct EwramData_EntityData *part;
    s32 i, facing;
    entity->updateFunc = (u32 *)EventScene03UnusedPart_Update;
    entity->posX.part16.integer = x;
    entity->posY.part16.integer = y;
    ENTITY_PARENT_INDEX(entity) = ENTITY_INDEX(owner);
    entity->ext.event03Part.flags = flags;
    entity->enemyId = 0xFF;
    Entity_SnapToGround(entity);
    Anim_StartIndexed(entity, 0, 0, 1);
    if (entity->ext.event03Part.flags & 4)
    {
        for (i = 16; i != 0; i--)
            entity->posY.whole += 0x8000;
        entity->spriteLayer = 0x17;
    }
    facing = 0;
    if (entity->posX.whole < gEwramData->playerState.player->posX.whole)
        facing = 1;
    entity->flipX = facing;
    part = Entity_CreateFromObjectTable(9);
    part->updateFunc = (u32 *)EventScene03AttachedEffect_Update;
    ENTITY_PARENT_INDEX(part) = ENTITY_INDEX(entity);
    part->posX.part16.integer = entity->posX.part16.integer;
    part->posY.part16.integer = entity->posY.part16.integer;
    Anim_StartIndexed(part, 16, 0, 1);
    part->spriteLayer = entity->spriteLayer;
    entity->ext.event03Part.linkIndex = ENTITY_INDEX(part);
    part->enemyId = entity->enemyId;
    return entity;
}
