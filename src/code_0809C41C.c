/* Stolas helpers 0x0809C41C..0x0809C5E7. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);
extern void Stolas_OnDamaged(struct EwramData_EntityData *entity, s32 attacker, u16 *hit, s16 damage);
extern void EntityDampVelocityX(struct EwramData_EntityData *entity, s32 factor);
extern void EntityDampVelocityY(struct EwramData_EntityData *entity, s32 factor);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *entity, s32 maxSpeed);
extern s32 Entity_GetVisibilityPattern(s32 step);

/* Graphics of Stolas. */
extern u8 sStolasGfx[];
extern u8 sStolasPalette[];
extern u8 sStolasSpriteSheet[];

void Stolas_Draw(struct EwramData_EntityData *entity);

/**
 * @brief 9C41C | Hit on a part: compute the damage at the part's position
 * for the body (unk_500), show the number and pass it to Stolas_OnDamaged.
 */
void StolasPart_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    struct EwramData_EntityData *stolas = entity->unk_500.unk_500_entity;
    s32 oldX = stolas->posX.whole;
    s32 oldY = stolas->posY.whole;
    s32 damage;

    stolas->posX.whole = entity->posX.whole;
    stolas->posY.whole = entity->posY.whole;
    damage = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(entity->unk_500.unk_500_entity, attacker, hit, (s16)damage);
    stolas->posX.whole = oldX;
    stolas->posY.whole = oldY;
    Entity_ShowDamage(entity, damage, 0);
    Stolas_OnDamaged(entity->unk_500.unk_500_entity, attacker, hit, damage);
}

/**
 * @brief 9C488 | Debris: slow down, fall, blink and delete after timer.
 */
void StolasDebris_Update(struct EwramData_EntityData *entity)
{
    u32 var_0[4];

    if (entity->state == 0)
    {
        EntityDampVelocityX(entity, 0x18);
        EntityDampVelocityY(entity, 0x18);
        EntityAccelerateAndMove(entity, 0x80000);
        entity->hidden = Entity_GetVisibilityPattern(entity->timer);
        if (--entity->timer == 0)
            EntityDelete(entity);
    }
}

/**
 * @brief 9C4E4 | Load the graphics of Stolas (drawn by Stolas_Draw); 0 when
 * no slot is free.
 */
u8 Stolas_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sStolasGfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sStolasPalette, 0);
    Sprite_Attach(entity, sStolasSpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Stolas_Draw;
    return 1;
}

/**
 * @brief Bestiary preview state (state 3): nothing.
 */
void Stolas_UpdatePreview(void)
{
}

/**
 * @brief 9C534 | Player_ApplyContactDamage with (attack, 0, 0, 1) at the entity position.
 */
void Stolas_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unk_4 = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 9C564 | Hit on the body: damage from hit, damage number, then
 * Stolas_OnDamaged.
 */
void Stolas_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s16 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    Entity_ShowDamage(entity, damage, 0);
    Stolas_OnDamaged(entity, attacker, hit, damage);
}

/**
 * @brief 9C5AC | Draw at the whole-pixel position offset by (unk_4F8,
 * unk_4FA), then restore posX / posY.
 */
void Stolas_Draw(struct EwramData_EntityData *entity)
{
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;

    entity->posX.part16.integer += entity->unk_4F8.part16.unk_4F8;
    entity->posX.part16.fraction = 0;
    entity->posY.part16.integer += entity->unk_4F8.part16.unk_4FA;
    entity->posY.part16.fraction = 0;
    Sprite_DrawEntity(entity);
    entity->posX.whole = oldX;
    entity->posY.whole = oldY;
}
