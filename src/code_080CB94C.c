/* Buer: load graphics, create its linked parts and set initial collision
 * and movement state. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *, void *, void *, s32, void *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Buer_CreateHead(struct EwramData_EntityData *);
extern void Buer_AllocateAffine(struct EwramData_EntityData *);
extern void Buer_DamagePlayerOnContact(struct EwramData_EntityData *);
extern void Buer_OnHit(struct EwramData_EntityData *);
extern u8 sBaelBuerGfx[], sBaelBuerPalette[], sBaelBuerSpriteSheet[];
static inline void SetState(struct EwramData_EntityData *entity, s8 value4F0, u8 state)
{
    entity->state = state;
    entity->stage = value4F0;
    entity->subState = 0;
}
void EnemyBuerCreate(struct EwramData_EntityData *entity)
{
    u8 *palette = sBaelBuerPalette;
    u8 *sprites = sBaelBuerSpriteSheet;
    u8 *graphics = sBaelBuerGfx;
    if (!Entity_LoadGraphics(entity, graphics, palette, 0, sprites))
    {
        EntityDelete(entity);
        return;
    }
    entity->spriteLayer = 20;
    Enemy_ResetFromTemplate(entity);
    entity->animFrame = 0;
    entity->scrollWithBg = 1;
    entity->ext.wheel.speed = 0x8000;
    entity->accelY.whole = 0x4000;
    if (entity->spawnParam0 == 0) SetState(entity, 0, 1);
    else SetState(entity, 0, 2);
    Buer_CreateHead(entity);
    Buer_AllocateAffine(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        SetState(entity, 0, 4);
    }
    else
    {
        Hit_Enable(entity, 8, Buer_DamagePlayerOnContact, (s32)Buer_OnHit);
        HitBox_SetAttackRaw(entity, 0x2828ECEC);
        HitBox_SetBodyRaw(entity, 0x2828ECEC);
        Entity_SnapToGround(entity);
        entity->posY.part16.integer -= 24;
    }
}
