/* Minotaur initialisation shared by Minotaur (variant 0) and Red Minotaur
 * (variant 1).  Loads graphics for the variant,
 * stores the variant in unk_4F9, creates the two helper entities MinotaurAxe_Update
 * and MinotaurSwingTrail_Update (any failure deletes the Minotaur), starts animation 1 and
 * either enters state 4 (bestiaryPreview set) or registers the hit/death callbacks
 * and enters state 0. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, u8 *, void *, u8, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Entity_EnableBoxes(struct EwramData_EntityData *, void *, void *);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *);
extern void Player_SetSelectedTarget(struct EwramData_EntityData *);
extern void MinotaurAxe_Update(struct EwramData_EntityData *);
extern void MinotaurSwingTrail_Update(struct EwramData_EntityData *);
extern void Minotaur_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Minotaur_OnHit(struct EwramData_EntityData *);
extern u8 sMinotaurGfx[], sMinotaurPalette[], sMinotaurSpriteSheet[];

/* Two child entities inherit the parent pointer, low unk_4F4 byte, attack
 * and the hp value of the Minotaur. */
static inline void AttachChild(struct EwramData_EntityData *child, struct EwramData_EntityData *entity)
{
    ENTITY_OWNER(child) = entity;
    ENTITY_PARENT_INDEX(child) = ENTITY_INDEX(entity);
    child->attack = entity->attack;
    /* Signed 16-bit view of hp; the u16 field allocates registers differently. */
    *(s16 *)&child->hp = *(s16 *)&entity->hp;
}

void Minotaur_Init(struct EwramData_EntityData *entity, s32 variant)
{
    struct EwramData_EntityData *child;
    if (!Entity_LoadGraphics(entity, sMinotaurGfx, sMinotaurPalette, variant, sMinotaurSpriteSheet))
        goto fail;
    entity->ext.minotaur.variant = variant;
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 0x13;
    child = Entity_CreateObject((u32 *)MinotaurAxe_Update);
    if (child == NULL)
        goto fail;
    AttachChild(child, entity);
    child = Entity_CreateObject((u32 *)MinotaurSwingTrail_Update);
    if (child == NULL)
    {
    fail:
        EntityDelete(entity);
        return;
    }
    AttachChild(child, entity);
    entity->ext.minotaur.flags |= 3;
    Anim_StartIndexed(entity, 1, 0, 1);
    Anim_Update(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        return;
    }
    Entity_EnableBoxes(entity, Minotaur_DamagePlayerOnContact, Minotaur_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->posY.part16.integer++;
    entity->scrollWithBg = 1;
    entity->state = 0;
    if (entity->spawnParam0 != 0)
        Player_SetSelectedTarget(entity);
}
