/* Disc Armor throw. */
#include "code/code_08039340.h"
#include "disc_armor_entity.h"
#include "structs/ewram.h"
extern void Entity_FaceTarget(struct EwramData_EntityData *);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *updateFunc);
extern void DiscArmorDisc_Update(struct EwramData_EntityData *);

/**
 * @brief B205C | Disc Armor throw (attack state).  State 0 faces the player,
 * starts animation 2 and sets unk_4F8 = 1 and the hitbox-like pair
 * unk_504/unk_506 = -20/-36.  State 1 spawns the disc DiscArmorDisc_Update at
 * animation frame 0x10C (owner in unk_4F4, same facing, attack * 1.25) and
 * switches to animation 3 / state 2 once unk_4F8 bit 1 is set (by the disc).
 * State 2 eases the pair back to -10/-16 and, when the animation reports
 * flag 4, clears unk_4F8 and the states and faces the player again.
 */
void DiscArmor_UpdateThrow(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *disc;

    switch (entity->subState)
    {
    case 0:
        Entity_FaceTarget(entity);
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->subState = 1;
        DISC_ARMOR_FLAGS(entity) = 1;
        DISC_ARMOR_HAND_OFFSET_X(entity) = -20;
        DISC_ARMOR_HAND_OFFSET_Y(entity) = -36;
        break;
    case 1:
        if (*(u16 *)&entity->animStep == 0x10C)
        {
            disc = Entity_CreateObject((u32 *)DiscArmorDisc_Update);
            if (disc != NULL)
            {
                DISC_ARMOR_DISC_OWNER(disc) = entity;
                disc->flipX = entity->flipX;
                disc->attack = entity->attack + (entity->attack >> 2);
            }
        }
        if (DISC_ARMOR_FLAGS(entity) & 2)
        {
            Anim_StartIndexed(entity, 3, 0, 0);
            entity->subState = 2;
        }
        break;
    case 2:
        if (DISC_ARMOR_HAND_OFFSET_X(entity) < -10)
            DISC_ARMOR_HAND_OFFSET_X(entity)++;
        if (DISC_ARMOR_HAND_OFFSET_Y(entity) < -16)
            DISC_ARMOR_HAND_OFFSET_Y(entity)++;
        if (entity->animFlags & 4)
        {
            DISC_ARMOR_FLAGS(entity) = 0;
            entity->state = 0;
            entity->stage = 0;
            entity->subState = 0;
            Entity_FaceTarget(entity);
        }
        break;
    }
}
