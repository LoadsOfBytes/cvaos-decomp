/* Death boss overlay part (DeathOverlay_Update): follows its owner, hidden while
 * the owner is in state 4 or flagged; deleted when the owner reaches state 7. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern u8 sDeathGfx[], sDeathPalette[], sDeathSpriteSheet[];

void DeathOverlay_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = &gEwramData->entityData[(u8)entity->ext.deathHelper.ownerIndex];

    if (owner->state == 7)
        goto delete;
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sDeathGfx, sDeathPalette, 0, sDeathSpriteSheet))
        {
        delete:
            EntityDelete(entity);
            return;
        }
        entity->spriteLayer = owner->spriteLayer + 1;
        Anim_StartIndexed(entity, 0, 2, 1);
        entity->scrollWithBg = 1;
        entity->state = 1;
    case 1:
        if (owner->state == 4 || (owner->ext.death.flags & 0x80))
            entity->hidden = 1;
        else
            entity->hidden = 0;
        entity->posX.whole = owner->posX.whole;
        entity->posY.whole = owner->posY.whole;
        break;
    }
    entity->objMode = owner->objMode;
    { u8 flags = owner->palette; entity->palette = flags; }
    Anim_Update(entity);
}
