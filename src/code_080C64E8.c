/* Ukoback helpers 0x080C64E8..0x080C664B. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);
extern void Ukoback_UpdateDeath(struct EwramData_EntityData *entity);
extern s32 UkobackFireball_Initialize(struct EwramData_EntityData *entity, struct EwramData_EntityData *ukoback);
extern void Ukoback_SpawnContactSparkle(void);
extern void UkobackFireball_Update(struct EwramData_EntityData *entity);
extern void UkobackDeathPiece_Update(struct EwramData_EntityData *entity);

static inline void Entity_DamagePlayerOnContact(struct EwramData_EntityData *entity, u16 attributes, u16 param_2, u16 reaction)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = attributes;
    contactDamage.unk_4 = param_2;
    contactDamage.reaction = reaction;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief Start animation 0 once and then advance it. The gameplay purpose of state 1 remains unresolved.
 */
void Ukoback_UpdateState1Animation(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->subState++;
    }
    Anim_Update(entity);
}

/**
 * @brief Submit the entity attack contact record with mode fields 0, 0, 0.
 */
void Ukoback_OnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/**
 * @brief Resolve damage, subtract health, show damage and flash; start the death sequence on lethal damage.
 */
void Ukoback_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    {
        u16 hp = entity->unk_518.part16.hp;
        damage = (damage << 16) >> 16;
        entity->unk_518.part16.hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    entity->unk_510.part8.hitFlashTimer = 8;
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        entity->subState = 0;
        Ukoback_UpdateDeath(entity);
    }
}

/**
 * @brief Allocate and initialize a fireball; delete it if initialization fails and return 1 only on success.
 */
s32 Ukoback_SpawnFireball(struct EwramData_EntityData *entity)
{
    s32 created = 0;
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)UkobackFireball_Update);

    if (child != NULL)
    {
        if (!UkobackFireball_Initialize(child, entity))
            EntityDelete(child);
        else
            created = 1;
    }
    return created;
}

/**
 * @brief Submit the attack contact record, create ten player-following sparkles and enter dissolving state 0xFF with hit flash.
 */
void UkobackFireball_OnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;
    s32 i;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unk_4 = 0;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    for (i = 9; i >= 0; i--)
        Ukoback_SpawnContactSparkle();
    entity->unk_510.part8.hitFlashTimer = 8;
    entity->state = 0xFF;
}

/**
 * @brief Set an eight-tick hit flash and enter dissolving state 0xFF.
 */
void UkobackFireball_OnHit(struct EwramData_EntityData *entity)
{
    entity->unk_510.part8.hitFlashTimer = 8;
    entity->state = 0xFF;
}

/**
 * @brief Create a falling death piece with the owner slot stored in its parent field; return 1 on success, otherwise 0.
 */
s32 Ukoback_SpawnDeathPiece(struct EwramData_EntityData *entity)
{
    s32 created = 0;
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)UkobackDeathPiece_Update);

    if (child != NULL)
    {
        child->unk_4F4.part16.unk_4F6 = entity->unk_4F4.part16.unk_4F4;
        created = 1;
    }
    return created;
}

/* Nemesis create and graphics loader 0x080C664C..0x080C67CB. The loader only matches without
 * -f2003-patch. */
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern s32 Hit_Enable(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);
extern s32 HitBox_SetAttackRaw(struct EwramData_EntityData *entity, s32 box);
extern s32 HitBox_SetBodyRaw(struct EwramData_EntityData *entity, s32 box);
extern s32 Nemesis_CreatePartner(struct EwramData_EntityData *parent);
extern void Nemesis_OnContact(struct EwramData_EntityData *entity);
extern void Nemesis_OnHit(struct EwramData_EntityData *param_0);
extern void Nemesis_UpdateAppearance(struct EwramData_EntityData *entity);
extern void Nemesis_CreateAfterimages(struct EwramData_EntityData *entity);
extern void Nemesis_SetAfterimagesHidden(struct EwramData_EntityData *entity, s32 hidden);
s32 Nemesis_LoadGraphics(struct EwramData_EntityData *entity);

/* Graphics of the Nemesis (Nemesis_LoadGraphics). */
extern u8 sNemesisGfx[];
extern u8 sNemesisPalette[];
extern u8 sNemesisSpriteSheet[];

/**
 * @brief C664C | Nemesis create: load the graphics (delete the enemy when
 * that fails), hit boxes, layer 3; state 7 when bestiaryPreview is set.
 * Otherwise ends (0x53D bit 3) when Nemesis_CreatePartner fails, sets the hit box
 * callbacks and boxes, unk_4FA 0, unk_4FB 9, unk_508 3, faces left and
 * runs Nemesis_UpdateAppearance, Nemesis_CreateAfterimages and Nemesis_SetAfterimagesHidden(1).
 */
void EnemyNemesisCreate(struct EwramData_EntityData *entity)
{
    if (!Nemesis_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 3;
    if (gEwramData->gameState.bestiaryPreview)
    {
        if (entity->state != 7)
        {
            entity->state = 7;
            entity->stage = 0;
            entity->subState = 0;
        }
        return;
    }
    if (!Nemesis_CreatePartner(entity))
        entity->deleteRequested = 1;
    Hit_Enable(entity, 8, Nemesis_OnContact, (s32)Nemesis_OnHit);
    HitBox_SetAttackRaw(entity, 0x080804FC);
    HitBox_SetBodyRaw(entity, 0x1010F8F8);
    entity->animFrame = 0;
    entity->unk_4F8.part8.unk_4FA = 0;
    entity->unk_4F8.part8.unk_4FB = 9;
    entity->unk_508.part8.unk_508 = 3;
    entity->flipX = 0;
    Nemesis_UpdateAppearance(entity);
    Nemesis_CreateAfterimages(entity);
    Nemesis_SetAfterimagesHidden(entity, 1);
}

/**
 * @brief Find or load graphics and palette, attach the sprite and allocate an affine slot; return 0 on failure, 1 on success.
 */
s32 Nemesis_LoadGraphics(struct EwramData_EntityData *entity)
{
    u8 *graphics = sNemesisGfx;
    s32 gfx = ObjGfx_Find(graphics);
    s8 paletteIndex;
    s32 slot;

    if (gfx < 0)
    {
        gfx = ObjGfx_LoadResource(graphics, 0);
        if (gfx < 0)
            return 0;
    }
    paletteIndex = ObjPalette_FindOrLoadVariant(sNemesisPalette, 0);
    if (paletteIndex == -1)
        return 0;
    entity->unk_510.part8.basePalette = paletteIndex;
    if (!Sprite_Attach(entity, sNemesisSpriteSheet, gfx, entity->unk_510.part8.basePalette))
        return 0;
    slot = ObjAffine_Alloc(1);
    if (slot < 0)
        return 0;
    entity->affineIndex = slot;
    entity->affine = 1;
    ObjAffine_Set(0, 0x10000, 0x10000, 0, 0, entity->affineIndex);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

extern void Nemesis_UpdateState7(struct EwramData_EntityData *entity);
extern void Nemesis_UpdateDeath(struct EwramData_EntityData *entity);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *entity);
extern u8 Entity_IsTargetWithinRange(struct EwramData_EntityData *entity, s32 range);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern void Nemesis_UpdatePositioning(struct EwramData_EntityData *entity);
extern void Nemesis_UpdateAttackSelection(struct EwramData_EntityData *entity);
extern void Nemesis_UpdateTurningAttack(struct EwramData_EntityData *entity);
extern void Nemesis_UpdateScriptedAttack(struct EwramData_EntityData *entity);
extern void Nemesis_UpdateSpinLunge(struct EwramData_EntityData *entity);
extern void Nemesis_UpdatePartnerFlight(struct EwramData_EntityData *entity);

/**
 * @brief C67CC | Nemesis update: state 7 Nemesis_UpdateState7, 0xFF Nemesis_UpdateDeath.
 * Otherwise Player_TrackEntity; when Enemy_IsStunned holds, it waits in state 0
 * unless Entity_IsTargetWithinRange(0x50 pixels) and else only collides (unless unk_4FC
 * bit 5). Runs the state (0..6), then collision unless unk_4FC bit 5.
 */
void EnemyNemesisUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];

    if (entity->state == 7)
    {
        Nemesis_UpdateState7(entity);
        return;
    }
    if (entity->state == 0xFF)
    {
        Nemesis_UpdateDeath(entity);
        return;
    }
    Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity))
    {
        if (!Entity_IsTargetWithinRange(entity, 0x500000) && entity->state == 0)
            return;
        if (!(entity->unk_4FC.part16.unk_4FC & 0x20) && entity->state != 0)
        {
            Hit_CheckWithPlayer(collision, entity);
            return;
        }
    }
    switch (entity->state)
    {
        case 0:
            Nemesis_UpdateAppearance(entity);
            break;
        case 1:
            Nemesis_UpdatePositioning(entity);
            break;
        case 2:
            Nemesis_UpdateAttackSelection(entity);
            break;
        case 3:
            Nemesis_UpdateTurningAttack(entity);
            break;
        case 4:
            Nemesis_UpdateScriptedAttack(entity);
            break;
        case 5:
            Nemesis_UpdateSpinLunge(entity);
            break;
        case 6:
            Nemesis_UpdatePartnerFlight(entity);
            break;
    }
    if (!(entity->unk_4FC.part16.unk_4FC & 0x20))
        Hit_CheckWithPlayer(collision, entity);
}

/* (u8)(t % 3) != 0 (hidden on two of three frames), written so that agbcc
 * emits `negs` + `lsrs #31` like the original. */
#define IS_BLINK_FRAME(t) (-(s32)(u8)((t) % 3) < 0)

/**
 * @brief Hide the entity and partner until the proximity check succeeds, then run the timed blinking appearance sequence and enter state 1.
 */
void Nemesis_UpdateAppearance(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *other = &gEwramData->entityData[entity->unk_4F4.part8.unk_4F6];
    s32 x;

    switch (entity->subState)
    {
        case 0:
            entity->unk_500.part16.unk_500 = 0x1000;
            entity->hidden = 1;
            other->hidden = 1;
            Nemesis_SetAfterimagesHidden(entity, 1);
            other->state = 5;
            entity->timer = 0;
            entity->subState++;
        case 1:
            if (!Entity_IsTargetWithinRange(entity, 0x500000))
                break;
            x = entity->posX.whole;
            other->posX.whole = x + 0xA0000;
            if (!other->flipX)
                other->posX.whole = x + 0xD0000;
            other->posY.whole = entity->posY.whole + 0x100000;
            ObjAffine_Set(entity->unk_500.part16.unk_500, 0x10000, 0x10000, 0, 0, entity->affineIndex);
            entity->subState++;
            break;
        case 2:
            entity->hidden = IS_BLINK_FRAME(entity->timer);
            other->hidden = IS_BLINK_FRAME(entity->timer);
            entity->objMode = 1;
            if (entity->timer++ > 40)
            {
                entity->hidden = 0;
                other->hidden = 0;
                Nemesis_SetAfterimagesHidden(entity, 0);
                entity->objMode = 0;
                entity->timer = 0;
                entity->subState++;
            }
            break;
        case 3:
            if (entity->timer++ > 20)
            {
                entity->timer = 0;
                entity->subState++;
            }
            break;
        case 4:
            other->hidden = IS_BLINK_FRAME(entity->timer);
            if (entity->timer++ > 40)
            {
                other->hidden = 1;
                if (other->state != 0)
                {
                    other->state = 0;
                    other->stage = 0;
                    other->subState = 0;
                }
                entity->timer = 0;
                entity->subState++;
            }
            break;
        case 5:
            if (entity->timer++ > 20)
            {
                entity->timer = 0;
                if (entity->state != 1)
                {
                    entity->state = 1;
                    entity->stage = 0;
                    entity->subState = 0;
                }
            }
            break;
    }
}
