/* Witch attached spell effect. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern u8 sWitchGfx[], sWitchPalette[], sWitchSpriteSheet[];

/**
 * @brief Follow the owner during its casting flag, with timed drawSkip
 * transitions. Begin removal when that flag clears or the owner dies.
 */
void WitchCastEffect_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[entity->unk_50C.part8.unk_50C];
    s32 timer;
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sWitchGfx, sWitchPalette, 0, sWitchSpriteSheet)) goto remove;
        entity->spriteLayer = 3;
        Anim_StartIndexed(entity, 5, 0, 1);
        entity->objMode = 1;
        entity->drawSkip = 1;
        entity->timer = 0x10;
        entity->state = 1;
        /* fall through */
    case 1:
        if (entity->timer != 0) entity->timer--;
        else
        {
            entity->drawSkip = 0;
            entity->state = 2;
        }
        break;
    case 2:
        if (!(parent->unk_4F8.part8.unk_4F8 & 2))
        {
            entity->state = 3;
            entity->timer = 0x10;
        }
        break;
    case 3:
        entity->drawSkip = 1;
        timer = entity->timer;
        if (timer == 0)
        {
remove:
            EntityDelete(entity);
            return;
        }
        entity->timer = timer - 1;
        break;
    }
    Anim_Update(entity);
    if (entity->state <= 2)
    {
        if (parent->state == 3)
        {
            entity->state = 3;
            entity->timer = 0x10;
        }
        else
        {
            entity->flipX = parent->flipX;
            entity->posX.whole = parent->posX.whole;
            entity->posY.whole = parent->posY.whole;
        }
    }
}
