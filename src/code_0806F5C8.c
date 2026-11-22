/* Spawn and position a Werewolf attack entity. */
#include "code_08009A0.h"
#include "structs/ewram.h"

extern void WerewolfShockwave_Update(struct EwramData_EntityData *);
extern u8 WerewolfShockwave_LoadGraphics(struct EwramData_EntityData *);
extern void WerewolfShockwave_EnableHitBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);

/**
 * @brief Spawn a ground shockwave offset px in front of parent, snapped to the
 * ground, with the parent's attack.
 */
void Werewolf_SpawnShockwave(struct EwramData_EntityData *entity, s32 offsetX)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x1B, 0x48, (u32 *)WerewolfShockwave_Update);
    if (child != NULL)
    {
        if (!WerewolfShockwave_LoadGraphics(child))
            EntityDelete(child);
        WerewolfShockwave_EnableHitBoxes(child);
        child->scrollWithBg = 1;
        child->spriteLayer = 0x12;
        ENTITY_CREATOR(child) = entity;
        if (entity->flipX)
            child->posX.part16.integer = entity->posX.part16.integer + offsetX;
        else
            child->posX.part16.integer = entity->posX.part16.integer - offsetX;
        child->posY.part16.integer = entity->posY.part16.integer;
        Entity_SnapToGround(child);
        child->posY.part16.integer++;
        child->flipX = entity->flipX;
        child->attack = entity->attack;
    }
}
