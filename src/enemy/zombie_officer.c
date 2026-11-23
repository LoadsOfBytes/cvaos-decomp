/*
 * Zombie Officer: body and pieces.
 */
#include "code_08009A0.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"
#include "code/code_08040A38.h"
#include "code_080D73B8.h"

extern s32 Enemy_PreUpdate(struct EwramData_EntityData *entity);
extern void Enemy_UpdateFragmentDeath(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);
extern void ZombieOfficer_UpdateWalk(struct EwramData_EntityData *entity);
extern void ZombieOfficer_UpdateAttack(struct EwramData_EntityData *entity);
extern void ZombieOfficer_SpawnPiece(struct EwramData_EntityData *entity, s32 x, s32 y, s32 vx, s32 vy, s32 frame);

/**
 * @brief 92174 | Zombie Officer update (as EnemyZombieSoldierUpdate):
 * states 0 / 1 (ZombieOfficer_UpdateWalk / ZombieOfficer_UpdateAttack), 2 dying: on frame 0x28 three
 * body parts ZombieOfficer_SpawnPiece (frames 0x10, 0x11 random, 0xF the head 6 px in
 * front and 13 px up), 3 animation only.
 */
void EnemyZombieOfficerUpdate(struct EwramData_EntityData *entity)
{
    u8 unused[0x10];
    s32 x, y, vx;

    if (Enemy_PreUpdate(entity))
        return;
    switch (entity->state)
    {
    case 0:
        ZombieOfficer_UpdateWalk(entity);
        break;
    case 1:
        ZombieOfficer_UpdateAttack(entity);
        break;
    case 2:
        if (entity->timer == 0x28)
        {
            x = (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
            y = -0x10000 - (RandomNumberGenerator() & 0xFFFFF);
            if (entity->flipX)
                vx = -0x8000 - (RandomNumberGenerator() & 0x7FFF);
            else
                vx = (RandomNumberGenerator() & 0x7FFF) + 0x8000;
            ZombieOfficer_SpawnPiece(entity, x, y, vx, -(RandomNumberGenerator() % 0x18000) - 0x18000, 0x10);
            x = (RandomNumberGenerator() & 0xFFFFF) - 0x80000;
            y = -(RandomNumberGenerator() % 0x180000);
            if (entity->flipX)
                vx = -0x8000 - (RandomNumberGenerator() & 0x7FFF);
            else
                vx = (RandomNumberGenerator() & 0x7FFF) + 0x8000;
            ZombieOfficer_SpawnPiece(entity, x, y, vx, -(RandomNumberGenerator() % 0x18000) - 0x18000, 0x11);
            x = entity->flipX ? 0x60000 : -0x60000;
            y = -0xD0000;
            if (entity->flipX)
                vx = -0x8000 - (RandomNumberGenerator() & 0x7FFF);
            else
                vx = (RandomNumberGenerator() & 0x7FFF) + 0x8000;
            ZombieOfficer_SpawnPiece(entity, x, y, vx, -0x10000 - (RandomNumberGenerator() & 0xFFFF), 0xF);
        }
        Enemy_UpdateFragmentDeath(entity);
        break;
    case 3:
        Anim_Update(entity);
        return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   EnemyZombieOfficerCreate = EnemySkeletonKnightCreate */

extern u8 ZombieOfficer_LoadGraphics(struct EwramData_EntityData *entity);
extern void ZombieOfficer_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void ZombieOfficer_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_UpdateBoxes(struct EwramData_EntityData *entity);
extern void Entity_SnapToGround(struct EwramData_EntityData *entity);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *entity);
extern s32 Hit_EnableContact(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);

/**
 * @brief 92350 | Same as EnemySkeletonKnightCreate.
 */
void EnemyZombieOfficerCreate(struct EwramData_EntityData *entity)
{
    if (!ZombieOfficer_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        return;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, ZombieOfficer_DamagePlayerOnContact, (s32)ZombieOfficer_OnHit);
    else
        Hit_Enable(entity, 8, ZombieOfficer_DamagePlayerOnContact, (s32)ZombieOfficer_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = 0;
}

/* Select an approach or attack on animation events, then apply movement. */
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s32, s32, s32, s32);
extern s32 Entity_MoveWithCollision(struct EwramData_EntityData *, s32, s32, s32, s32);
/**
 * @brief Walk state (state 0): on the step frames 3 / 7 face the target and attack
 * (state 1) when it is in the 100 x 70 box; otherwise keep walking.
 */
void ZombieOfficer_UpdateWalk(struct EwramData_EntityData *entity)
{
    if (entity->subState != 0) return;
    if ((entity->animStep == 3 || entity->animStep == 7) && entity->animTimer == 0)
    {
        entity->flipX = Entity_IsTargetRight(entity);
        if (Target_IsInRect((s16)(entity->posX.part16.integer - 50), (s16)(entity->posY.part16.integer - 64), 100, 70))
        {
            entity->state = 1;
            entity->subState = 0;
            return;
        }
        if (entity->flipX) entity->velX.whole = 0xA000;
        else entity->velX.whole = -0xA000;
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2800;
    }
    if ((entity->animStep == 2 || entity->animStep == 6) && entity->animTimer == 0) entity->velX.whole = 0;
    Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 10);
}

/* ZombieOfficer_UpdateAttack. */
/* animStep and animTimer read as one halfword (as in code_080756FC.c). */
#define FRAME_PAIR(e) (*(u16 *)&(e)->animStep)

/**
 * @brief 924AC | Attack: animation 1, sound 0x85 at frame 4; when it ends
 * go back to state 0 with animation 0 and a hop (velX +-0xA000 by facing,
 * velY 0x10000, accelY 0x2800).
 */
void ZombieOfficer_UpdateAttack(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            Anim_StartIndexed(entity, 1, 0, 0);
            entity->subState++;
            break;
        case 1:
            if (FRAME_PAIR(entity) == 4)
                PlaySong(0x85);
            if (entity->animDone)
            {
                Anim_StartIndexed(entity, 0, 0, 1);
                entity->state = 0;
                entity->subState = 0;
                if (entity->flipX)
                    entity->velX.whole = 0xA000;
                else
                    entity->velX.whole = -0xA000;
                entity->velY.whole = 0x10000;
                entity->accelY.whole = 0x2800;
            }
            break;
    }
}

/* ZombieOfficer_OnHit (hit with HP / death), generated with a template script from the
 * template of WingedSkeleton_OnHit and checked byte for byte. */

extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);

/**
 * @brief 9252C | Hit: flash (hitFlashTimer = 8), subtract the damage from the HP
 * and show it; at HP <= 0 die (Enemy_DropRewards, song 0x70, animation 2, palette into
 * 0x53E bits 3-6, timer 0x28, state 2). Same template as WingedSkeleton_OnHit.
 */
void ZombieOfficer_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;

        // (s16)damage, written so that agbcc sign-extends after the HP load
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x70);
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 2;
        entity->subState = 0;
    }
}

/* ZombieOfficer_SpawnPiece. */
extern void ZombieOfficerPiece_Update(struct EwramData_EntityData *entity);
extern u8 sZombieOfficerGfx[];
extern u8 sZombieOfficerSpriteSheet[];

/**
 * @brief 925B4 | Spawn a ZombieOfficerPiece_Update projectile at entity's position +
 * (x, y) with velocity (vx, vy), animFrame frame,
 * gravity 0x1800, entity's facing and palette, one layer below; unk_500
 * keeps entity's y.
 */
void ZombieOfficer_SpawnPiece(struct EwramData_EntityData *entity, s32 x, s32 y, s32 vx, s32 vy, s32 frame)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)ZombieOfficerPiece_Update);

    if (child != NULL)
    {
        s32 gfx;

        child->posX.whole = entity->posX.whole + x;
        child->posY.whole = entity->posY.whole + y;
        child->ext.zombiePiece.floorY = entity->posY.whole;
        child->flipX = entity->flipX;
        child->spriteLayer = entity->spriteLayer - 1;
        gfx = ObjGfx_Find(sZombieOfficerGfx);
        Sprite_Attach(child, sZombieOfficerSpriteSheet, gfx, entity->basePalette);
        child->animFrame = frame;
        child->accelY.whole = 0x1800;
        child->velX.whole = vx;
        child->velY.whole = vy;
        child->scrollWithBg = 1;
        child->drawFunc = (u32 *)Sprite_DrawEntity;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   ZombieOfficerPiece_Update = ZombieSoldierPiece_Update
 *   ZombieOfficer_LoadGraphics = Object25_LoadGraphics
 *   ZombieOfficer_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

extern void EntityAccelerateAndMove(struct EwramData_EntityData *entity, s32 maxSpeed);
extern u8 sZombieOfficerPalette[];
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

static inline void Entity_DamagePlayerOnContact(struct EwramData_EntityData *entity, u16 attributes, u16 unused, u16 reaction)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = attributes;
    contactDamage.unused = unused;
    contactDamage.reaction = reaction;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief 92660 | Same as ZombieSoldierPiece_Update.
 */
void ZombieOfficerPiece_Update(struct EwramData_EntityData *entity)
{
    EntityAccelerateAndMove(entity, 0x80000);
    if (entity->posY.whole > entity->ext.zombiePiece.floorY)
        EntityDelete(entity);
}

/**
 * @brief 92680 | Same as Object25_LoadGraphics.
 */
u8 ZombieOfficer_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 gfx = ObjGfx_LoadResource(sZombieOfficerGfx, 0);

    if (gfx < 0)
        return 0;
    entity->basePalette = ObjPalette_FindOrLoadVariant(sZombieOfficerPalette, 0);
    Sprite_Attach(entity, sZombieOfficerSpriteSheet, gfx, entity->basePalette);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}

/**
 * @brief 926CC | Same as Enemy_DamagePlayerOnContact.
 */
void ZombieOfficer_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}
