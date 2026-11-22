/* Short blinking effect over an owner slot. */
#include "code_08009A0.h"
#include "death_entity.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern u8 sDeathGfx[], sDeathPalette[], sDeathSpriteSheet[];

/**
 * @brief BCFC8 | One-shot effect drawn just above its owner (entity slot
 * unk_4F8, priority + 1).  State 0 loads graphics sDeathGfx /
 * sDeathPalette / sDeathSpriteSheet (palette 2), plays animation subState and
 * starts a 12-frame counter (timer) with drawSkip = 1; at 8 it sets
 * objMode = 1 and clears drawSkip, at 4 sets drawSkip again, and at 0 (or
 * in state 2) the entity is deleted.
 */
void DeathBlinkEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[(u8)DEATH_OWNER_INDEX(entity)];

    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sDeathGfx, sDeathPalette, 2, sDeathSpriteSheet))
        {
            EntityDelete(entity);
            return;
        }
        entity->spriteLayer = owner->spriteLayer + 1;
        Anim_StartIndexed(entity, entity->subState, 2, 0);
        Anim_Update(entity);
        entity->drawSkip = 1;
        entity->timer = 12;
        entity->state = 1;
    case 1:
        if (entity->timer == 8)
        {
            entity->objMode = 1;
            entity->drawSkip = 0;
        }
        if (entity->timer == 4)
            entity->drawSkip = 1;
        if (entity->timer != 0)
        {
            entity->timer--;
            break;
        }
    case 2:
        EntityDelete(entity);
        break;
    }
}
