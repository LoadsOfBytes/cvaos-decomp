/* Objects 0x25..0x28 0x08057BAC..0x08058507. */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);
extern s32 Entity_MoveWithCollisionDefault(struct EwramData_EntityData *entity, s32 landingVelocity, s32 ceilingOffset);
extern s32 Entity_GetVisibilityPattern(s32 step);
extern void Effect_SpawnRandom(s32 x, s32 y, s32 width, s32 height, s32 layer);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *entity);
extern void EntityMoveX(struct EwramData_EntityData *entity);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);

/* Animations of object 0x25 (Anim_Start). */
extern u8 sObject25AnimScript1[];
extern u8 sObject25AnimScript2[];
/* Graphics of object 0x25 (Object25_LoadGraphics). */
extern u8 sObject25Gfx[];
extern u8 sLegionObject25Palette[];
extern u8 sObject25SpriteSheet[];

/**
 * @brief 57BAC | Object 0x25, state 0: wait for 0x53D bit 0, then walk
 * right (falling, Entity_MoveWithCollisionDefault) until the variant's end x (world, BG 1);
 * substate 10 blinks with Effect_SpawnRandom effects and finally ends the object,
 * decrementing the spawner's counter (unk_500 -> unk_4FC).
 */
void Object25_UpdateState(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    struct EwramData_EntityData *spawner = entity->unk_500.unk_500_entity;
    u32 hitInfo[4];

    switch (entity->subState)
    {
        case 0:
            if (!entity->animDone)
                break;
            entity->subState = 1;
            Anim_Start(entity, (struct AnimScript *)sObject25AnimScript1, 0, 1);
            entity->velX.whole = 0x4000;
            entity->velY.whole = 0x40000;
            entity->spriteLayer--;
            break;
        case 1:
            entity->velX.whole = 0x4000;
            EntityMoveX(entity);
            entity->velX.whole = 0;
            if (entity->unk_514.part16.spawnParam0 != 2)
            {
                entity->velY.whole = 0x40000;
                Entity_MoveWithCollisionDefault(entity, 0x40000, -0x200000);
            }
            else if (entity->posX.whole + bg->xPos.whole <= 0xD00000)
            {
                entity->velY.whole = 0x40000;
                Entity_MoveWithCollisionDefault(entity, 0x40000, -0x200000);
            }
            Enemy_UpdateHitFlash(entity);
            switch (entity->unk_514.part16.spawnParam0)
            {
                case 0:
                case 1:
                    if (entity->posX.whole + bg->xPos.whole > 0x40FFFFF)
                    {
                        entity->subState = 10;
                        entity->timer = 1;
                    }
                    break;
                case 2:
                case 3:
                    if (entity->posX.whole + bg->xPos.whole > 0xFFFFFF)
                    {
                        entity->subState = 10;
                        entity->timer = 1;
                    }
                    break;
            }
            break;
        case 10:
            if ((entity->timer & 3) == 0)
                Effect_SpawnRandom(entity->posX.whole, entity->posY.whole - 0x60000, 0x100000, 0xC0000, entity->spriteLayer - 1);
            entity->hidden = Entity_GetVisibilityPattern(entity->timer);
            if ((s8)--entity->timer > 0)
                break;
            entity->deleteRequested = 1;
            if (spawner != NULL && spawner->updateFunc != NULL && spawner->unk_4FC.whole != 0)
                spawner->unk_4FC.whole--;
            return;
    }
    Anim_Update(entity);
    if (entity->subState == 1)
        Hit_CheckWithPlayer(hitInfo, entity);
}

/**
 * @brief 57D38 | Object 0x25 hit: apply the damage of hit to the HP in
 * unk_518; at 0 switch to substate 10 (dying) with animation sObject25AnimScript2.
 */
void Object25_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    entity->unk_510.part8.hitFlashTimer = 8;
    // (s16)damage, written so that agbcc sign-extends in place
    damage <<= 16;
    damage >>= 16;
    Entity_ShowDamage(entity, damage, 0);
    entity->unk_518.part16.hp -= damage;
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 0x28;
        entity->unk_510.part8.hitFlashTimer = 0;
        entity->subState = 10;
        Anim_Start(entity, (struct AnimScript *)sObject25AnimScript2, 2, 1);
    }
}

/**
 * @brief 57DB8 | Load the graphics of object 0x25; 0 when no slot is free.
 * The same code exists 28 times in the ROM with other data.
 */
u8 Object25_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject25Gfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sLegionObject25Palette, 0);
    Sprite_Attach(entity, sObject25SpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/* Target_IsInRect (code_0806CF2C.c) takes u16 there; the callers here pass
 * sign-extended values. */
extern s32 Target_IsInRect(s16 left, s16 top, s16 width, s16 height);
void Object27Create(struct EwramData_EntityData *entity);
void Object27Update(struct EwramData_EntityData *entity);
void Object28Create(struct EwramData_EntityData *entity);
void Object28Update(struct EwramData_EntityData *entity);
u8 Object26_LoadGraphics(struct EwramData_EntityData *entity);
void Object26_UpdateState(struct EwramData_EntityData *entity);

/* Graphics of object 0x26 (Object26_LoadGraphics). */
extern u8 sObject13FamilyGfx[];
extern u8 sUnk_08209FD8[];
extern u8 sObject13FamilySpriteSheet[];

/**
 * @brief 57E04 | Object 0x26 create: load the graphics (delete the object
 * when that fails), set up and spawn objects 0x28 (-0x15, +0x44) and 0x27
 * (+0x12, +0x2C) linked back through their unk_4F8.
 */
void Object26Create(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child;

    if (!Object26_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    Anim_StartIndexed(entity, 3, 0, 1);
    Anim_Update(entity);
    entity->spriteLayer = 0x12;
    entity->flipX = 0;
    entity->scrollWithBg = 1;
    entity->hidden = 1;
    entity->unk_4FC.part16.unk_4FE[0] = 0x5A;
    entity->unk_4FC.part16.unk_4FC = 0x1E;
    entity->state = 0;
    child = EntityCreateInRange(0x1B, 0x48, (u32 *)Object28Update);
    if (child != NULL)
    {
        child->posX.part16.integer = entity->posX.part16.integer - 0x15;
        child->posY.part16.integer = entity->posY.part16.integer + 0x44;
        Object28Create(child);
        child->unk_4F8.unk_4F8_entity = entity;
    }
    child = EntityCreateInRange(0x1B, 0x48, (u32 *)Object27Update);
    if (child != NULL)
    {
        child->posX.part16.integer = entity->posX.part16.integer + 0x12;
        child->posY.part16.integer = entity->posY.part16.integer + 0x2C;
        Object27Create(child);
        child->unk_4F8.unk_4F8_entity = entity;
    }
}

/**
 * @brief 57EE0 | Object 0x26 update.
 */
void Object26Update(struct EwramData_EntityData *entity)
{
    u32 var_0[4];

    if (entity->state == 0)
        Object26_UpdateState(entity);
    Anim_Update(entity);
}

/**
 * @brief 57F00 | Load the graphics of object 0x26; 0 when no slot is free.
 */
u8 Object26_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject13FamilyGfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sUnk_08209FD8, 2);
    Sprite_Attach(entity, sObject13FamilySpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 57F4C | Object 0x26, state 0: while Target_IsInRect sees the area
 * (x-0x46, y-0x64, 0x8C x 0xC8) count up unk_4FC / unk_4FE; 0x53C bit 5
 * is set either way.
 */
void Object26_UpdateState(struct EwramData_EntityData *entity)
{
    if (entity->subState != 0)
        return;
    if (Target_IsInRect(entity->posX.part16.integer - 0x46, entity->posY.part16.integer - 0x64, 0x8C, 0xC8))
    {
        entity->hidden = 1;
        entity->unk_4FC.part16.unk_4FC++;
        entity->unk_4FC.part16.unk_4FE[0]++;
    }
    else
    {
        entity->hidden = 1;
    }
}

extern s32 Hit_Enable(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);
extern s32 HitBox_SetAttack(struct EwramData_EntityData *entity, void *box);
u8 Object28_LoadGraphics(struct EwramData_EntityData *entity);
void Object28_UpdateState(struct EwramData_EntityData *entity);
void Object28_OnPlayerContact(struct EwramData_EntityData *entity);

/**
 * @brief 57FA8 | Object 0x28 create: load the graphics, register the hit
 * box (Object28_OnPlayerContact on contact), damage from the player's stats, allocate
 * an affine slot (delete the object when anything fails).
 */
void Object28Create(struct EwramData_EntityData *entity)
{
    struct EwramData **ewramRef = &gEwramData;
    u16 affineIndex;
    u16 attack;

    if (!Object28_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Hit_Enable(entity, 8, Object28_OnPlayerContact, 0);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    Anim_StartIndexed(entity, 4, 0, 1);
    Anim_Update(entity);
    // Same contact damage formula as in Player_ApplyContactDamage.
    attack = ((*ewramRef)->playerState.equipmentDefenseBonus >> 1) + ((*ewramRef)->playerState.equipmentDefenseBonus >> 2)
        + ((*ewramRef)->playerStatus.maxHP >> 4);
    entity->attack = attack;
    entity->spriteLayer = 0x12;
    entity->flipX = 0;
    entity->scrollWithBg = 1;
    entity->hidden = 1;
    affineIndex = ObjAffine_Alloc(1);
    if ((s16)affineIndex < 0)
    {
        EntityDelete(entity);
        return;
    }
    entity->affineIndex = affineIndex;
    entity->affine = 1;
    ObjAffine_SetBiased(0, 0, 0, entity->flipX, entity->flipY, entity->affineIndex);
    entity->state = 0;
}

/**
 * @brief 58094 | Object 0x28, state 0: once the parent (unk_4F8) counted
 * 0x78 frames in unk_4FE, appear and grow (affine scale in velX) from
 * 0xC000 to 0x10000, stay with an active hit box for 30 frames, shrink back
 * and hide again.
 */
void Object28_UpdateState(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *controller = entity->unk_4F8.unk_4F8_entity;
    s32 zero = 0;
    u32 hitBox = 0x1010FCF8;

    switch (entity->subState)
    {
        case 0:
            if (controller->unk_4FC.part16.unk_4FE[0] < 0x78)
                break;
            controller->unk_4FC.part16.unk_4FE[0] = zero;
            entity->subState++;
            entity->hidden = 0;
            entity->velX.whole = 0xC000;
            break;
        case 1:
            if (entity->velX.whole < 0x10000)
            {
                entity->velX.whole += 0x800;
            }
            else
            {
                entity->velX.whole = 0x10000;
                entity->subState++;
                entity->timer = 0x1E;
                HitBox_SetAttack(entity, &hitBox);
                entity->hit.flags &= ~2;
            }
            ObjAffine_SetBiased(0, entity->velX.whole, entity->velX.whole, entity->flipX, entity->flipY, entity->affineIndex);
            break;
        case 2:
            if (entity->timer-- > 1)
                break;
            entity->subState++;
            entity->hit.flags |= 2;
            break;
        case 3:
            if (entity->velX.whole > 0xC000)
            {
                entity->velX.whole -= 0x800;
            }
            else
            {
                entity->subState = zero;
                entity->hidden = 1;
            }
            ObjAffine_SetBiased(0, entity->velX.whole, entity->velX.whole, entity->flipX, entity->flipY, entity->affineIndex);
            break;
    }
}

/**
 * @brief 581B4 | Object 0x28 update.
 */
void Object28Update(struct EwramData_EntityData *entity)
{
    u32 hitInfo[4];

    if (entity->state == 0)
        Object28_UpdateState(entity);
    Anim_Update(entity);
    Hit_CheckWithPlayer(hitInfo, entity);
}

/**
 * @brief 581DC | Same as Object26_LoadGraphics.
 */
u8 Object28_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject13FamilyGfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sUnk_08209FD8, 2);
    Sprite_Attach(entity, sObject13FamilySpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 58228 | Contact callback: Player_ApplyContactDamage with (attack, 0, 0, 1) at
 * the object position.
 */
void Object28_OnPlayerContact(struct EwramData_EntityData *entity)
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

void Object27_UpdateState(struct EwramData_EntityData *entity);
void Object27_OnPlayerContact(struct EwramData_EntityData *entity);
u8 Object27_LoadGraphics(struct EwramData_EntityData *entity);

/**
 * @brief 58258 | Object 0x27 create: same as Object28Create with animation 2.
 */
void Object27Create(struct EwramData_EntityData *entity)
{
    struct EwramData **ewramRef = &gEwramData;
    u16 affineIndex;
    u16 attack;

    if (!Object27_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Hit_Enable(entity, 8, Object27_OnPlayerContact, 0);
    entity->hit.flags |= 4;
    entity->hit.flags |= 2;
    Anim_StartIndexed(entity, 2, 0, 1);
    Anim_Update(entity);
    // Same contact damage formula as in Player_ApplyContactDamage.
    attack = ((*ewramRef)->playerState.equipmentDefenseBonus >> 1) + ((*ewramRef)->playerState.equipmentDefenseBonus >> 2)
        + ((*ewramRef)->playerStatus.maxHP >> 4);
    entity->attack = attack;
    entity->spriteLayer = 0x12;
    entity->flipX = 0;
    entity->scrollWithBg = 1;
    entity->hidden = 1;
    affineIndex = ObjAffine_Alloc(1);
    if ((s16)affineIndex < 0)
    {
        EntityDelete(entity);
        return;
    }
    entity->affineIndex = affineIndex;
    entity->affine = 1;
    ObjAffine_SetBiased(0, 0, 0, entity->flipX, entity->flipY, entity->affineIndex);
    entity->state = 0;
}

/**
 * @brief 58344 | Object 0x27, state 0: same as Object28_UpdateState with the parent
 * counter unk_4FC and a bigger hit box.
 */
void Object27_UpdateState(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *controller = entity->unk_4F8.unk_4F8_entity;
    s32 zero = 0;
    u32 hitBox = 0x2010F0F8;

    switch (entity->subState)
    {
        case 0:
            if (controller->unk_4FC.part16.unk_4FC < 0x78)
                break;
            controller->unk_4FC.part16.unk_4FC = zero;
            entity->subState++;
            entity->hidden = 0;
            entity->velX.whole = 0xC000;
            break;
        case 1:
            if (entity->velX.whole < 0x10000)
            {
                entity->velX.whole += 0x800;
            }
            else
            {
                entity->velX.whole = 0x10000;
                entity->subState++;
                entity->timer = 0x1E;
                HitBox_SetAttack(entity, &hitBox);
                entity->hit.flags &= ~2;
            }
            ObjAffine_SetBiased(0, entity->velX.whole, entity->velX.whole, entity->flipX, entity->flipY, entity->affineIndex);
            break;
        case 2:
            if (entity->timer-- > 1)
                break;
            entity->subState++;
            entity->hit.flags |= 2;
            break;
        case 3:
            if (entity->velX.whole > 0xC000)
            {
                entity->velX.whole -= 0x800;
            }
            else
            {
                entity->subState = zero;
                entity->hidden = 1;
            }
            ObjAffine_SetBiased(0, entity->velX.whole, entity->velX.whole, entity->flipX, entity->flipY, entity->affineIndex);
            break;
    }
}

/**
 * @brief 58464 | Object 0x27 update.
 */
void Object27Update(struct EwramData_EntityData *entity)
{
    u32 hitInfo[4];

    if (entity->state == 0)
        Object27_UpdateState(entity);
    Anim_Update(entity);
    Hit_CheckWithPlayer(hitInfo, entity);
}

/**
 * @brief 5848C | Same as Object26_LoadGraphics.
 */
u8 Object27_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sObject13FamilyGfx, 0);

    if (gfx < 0)
        return 0;
    entity->unk_510.part8.basePalette = ObjPalette_FindOrLoadVariant(sUnk_08209FD8, 2);
    Sprite_Attach(entity, sObject13FamilySpriteSheet, gfx, entity->unk_510.part8.basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 584D8 | Same as Object28_OnPlayerContact.
 */
void Object27_OnPlayerContact(struct EwramData_EntityData *entity)
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
