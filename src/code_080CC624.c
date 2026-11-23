/* Bael creation: graphics shared with Buer, affine slot, hitbox and callbacks. */
#include "code_08009A0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *graphics, void *palette, u8 slot, void *sprite);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern void Bael_CreateHead(struct EwramData_EntityData *entity);
extern void Bael_DamagePlayerOnContact(struct EwramData_EntityData *entity);
extern void Bael_OnHit(struct EwramData_EntityData *entity);
extern void Hit_Enable(struct EwramData_EntityData *entity, s32 mode, void *callback, s32 flags);
extern void HitBox_SetAttackRaw(struct EwramData_EntityData *entity, s32 flags);
extern s32 HitBox_SetBodyRaw(struct EwramData_EntityData *entity, s32 flags);
extern u8 sBaelBuerPalette[];
extern u8 sBaelBuerSpriteSheet[];
extern u8 sBaelBuerGfx[];
static inline void SetAffine(struct EwramData_EntityData *e, s32 index) { e->affineIndex = index; }

void EnemyBaelCreate(struct EwramData_EntityData *entity)
{
    void *palette = sBaelBuerPalette;
    void *sprite = sBaelBuerSpriteSheet;
    u8 *graphics = sBaelBuerGfx;
    s32 zero;
    u16 affine;
    u8 *layer;
    if (!Entity_LoadGraphics(entity, graphics, palette, 1, sprite))
    {
        EntityDelete(entity);
        return;
    }
    layer = &entity->spriteLayer;
    zero = 0;
    *layer = 20;
    Enemy_ResetFromTemplate(entity);
    entity->animFrame = zero;
    entity->scrollWithBg = 1;
    entity->ext.wheel.speed = 0x8000;
    entity->accelY.whole = 0x4000;
    if (entity->spawnParam0 == 0)
        entity->state = 1;
    else
        entity->state = 2;
    entity->stage = zero;
    entity->subState = zero;
    Bael_CreateHead(entity);
    affine = ObjAffine_Alloc(1);
    if ((s16)affine < 0)
    {
        EntityDelete(entity);
        return;
    }
    SetAffine(entity, affine);
    entity->affine = 1;
    {
        /* the scale in a local and the reloaded zero fix the order of "movs r4, #0" */
        s32 scale = 0x10000;
        zero = 0;
        ObjAffine_Set(scale, scale, scale, 0, 0, entity->affineIndex);
    }
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        entity->stage = zero;
        entity->subState = zero;
        return;
    }
    Hit_Enable(entity, 8, Bael_DamagePlayerOnContact, (s32)Bael_OnHit);
    HitBox_SetAttackRaw(entity, 0x2828ECEC);
    HitBox_SetBodyRaw(entity, 0x2828ECEC);
    Entity_SnapToGround(entity);
    entity->posY.part16.integer -= 24;
}
