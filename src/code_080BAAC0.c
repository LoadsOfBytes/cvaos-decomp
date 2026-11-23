/* Man Eater creation and attached effects. */
#include "code_08009A0.h"
#include "code_08013960.h"
#include "code_08033CAC.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void Effect_SpawnRandom(s32, s32, s32, s32, s32);
extern struct EwramData_EntityData *Entity_CreateObject(void *);
extern u8 sManEaterGfx[], sManEaterPalette[], sManEaterSpriteSheet[];
extern const u8 sUnk_085288A8[][4];

/**
 * @brief Follow the root owner using its affine slot; animate the indexed
 * part during head creation, then clear the shared affine reference and
 * delete without freeing the owner slot.
 */
void ManEaterAttachedPart_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *parent = &gEwramData->entityData[(u8)entity->ext.manEater.ownerIndex];
    const u8 *entry = sUnk_085288A8[entity->ext.manEater.orbitSlot];
    switch (entity->state)
    {
    case 0:
        if (!Entity_LoadGraphics(entity, sManEaterGfx, sManEaterPalette, 0, sManEaterSpriteSheet)) goto remove;
        entity->affineIndex = parent->affineIndex;
        entity->affine = 1;
        entity->animFrame = entry[0];
        entity->spriteLayer = 0x12;
        entity->state = 1;
        /* fall through */
    case 1:
        if (parent->state == 1 && parent->timer == 0x30 && entity->ext.manEater.orbitSlot == parent->subState)
        {
            Anim_StartIndexed(entity, entry[1], 0, 1);
            entity->state = 2;
            entity->timer = 0x30;
        }
        break;
    case 2:
        Anim_Update(entity);
        if (entity->timer != 0) entity->timer--;
        else entity->state = 3;
        break;
    case 3:
        entity->affineIndex = 0;
        entity->affine = 0;
remove:
        EntityDelete(entity);
        return;
    }
    entity->posX.whole = parent->posX.whole;
    entity->posY.whole = parent->posY.whole;
}

/**
 * @brief Damp and move a debris sprite, optionally emit random effects,
 * and request deletion at the end of its countdown.
 */
void ManEaterDebris_Update(struct EwramData_EntityData *entity)
{
    EntityDampVelocityX(entity, 0x50);
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->timer == 0xA) entity->drawSkip = 1;
    if (entity->subState == 0 && !(entity->timer & 0xF))
        Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0xC0000, 0xC0000, entity->spriteLayer - 1);
    if (--entity->timer == 0) entity->deleteRequested = 1;
}

