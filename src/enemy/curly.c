/*
 * Curly. The last part of the range also holds the entry points of Durga.
 */
#include "core.h"
#include "graphics.h"
#include "sound.h"
#include "structs/ewram.h"
#include "bg_and_collision.h"
#include "player_core.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "math_anim.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "damage_numbers.h"
#include "entity_motion.h"
#include "hitbox.h"

extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s32 Collision_IsWalkBlocked(s32 x, s32 y);
extern s16 Hit_ResolveDamage(struct EwramData_EntityData *, s32, u16 *, s32);

u8 Curly_LoadGraphics(struct EwramData_EntityData *);
void Curly_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void Curly_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);

void EnemyCurlyCreate(struct EwramData_EntityData *entity)
{
    if (!Curly_LoadGraphics(entity))
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
        Hit_EnableContact(entity, 8, Curly_DamagePlayerOnContact, Curly_OnHit);
    else
        Hit_Enable(entity, 8, Curly_DamagePlayerOnContact, Curly_OnHit);
    Entity_UpdateBoxes(entity);
    Entity_SnapToGround(entity);
    entity->state = 0;
    entity->velY.whole = 0x10000;
    entity->accelY.whole = 0x2800;
}

/**
 * @brief 93218 | Patrol AI: walks at 0x8000 (Entity_AdjustSpeedForSlope) while the
 * animation is outside frames 4-7 (stands still in them unless animTimer),
 * plays sound 0x85 on frame 4 and turns around on walls, ledges
 * (Collision_IsWalkBlocked one pixel ahead) or Entity_GetHorizontalRoomEdge; state 1 (after
 * Enemy_IsDamaged) always faces the player. In frame 1 with the player in the
 * 0x64 x 0x46 box it attacks (state = 1) with a 3/4 chance.
 */
void Curly_UpdateWalk(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 res;
    s32 x;
    s32 y;
    u8 anim;

    switch (entity->subState)
    {
    case 0:
        if (Enemy_IsDamaged(entity))
        {
            entity->subState++;
            break;
        }
        if ((u8)(entity->animStep - 4) > 3)
        {
            if (!entity->animTimer)
            {
                if (entity->flipX)
                    entity->velX.whole = Entity_AdjustSpeedForSlope(entity, 0x8000);
                else
                    entity->velX.whole = Entity_AdjustSpeedForSlope(entity, -0x8000);
                if ((anim = entity->animStep) == 1
                    && Target_IsInRect(entity->posX.part16.integer - 0x32, entity->posY.part16.integer - 0x40, 0x64, 0x46)
                    && (RandomNumberGenerator() & 0x7F) <= 0x5F)
                {
                    entity->state = anim;
                    entity->subState = 0;
                    entity->timer = 0;
                    Anim_StartIndexed(entity, 1, 0, 0);
                    break;
                }
            }
        }
        else if (!entity->animTimer)
        {
            entity->velX.whole = 0;
        }
        if (*(u16 *)&entity->animStep == 4)
            PlaySong(0x85);
        res = Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 8);
        if (!(res & 4))
            break;
        x = (s16)entity->posX.part16.integer + (s16)bg->xPos.part16.integer;
        y = (s16)entity->posY.part16.integer + (s16)bg->yPos.part16.integer;
        if (entity->flipX)
            x++;
        else
            x--;
        if ((res & 3) || Collision_IsWalkBlocked(x, y) || Entity_GetHorizontalRoomEdge(entity, 1))
        {
            entity->flipX ^= 1;
            if (entity->flipX)
                entity->velX.whole = Entity_AdjustSpeedForSlope(entity, 0x8000);
            else
                entity->velX.whole = Entity_AdjustSpeedForSlope(entity, -0x8000);
        }
        break;
    case 1:
        if ((u8)(entity->animStep - 4) > 3)
        {
            if (!entity->animTimer)
            {
                entity->flipX = Entity_IsTargetRight(entity);
                if (entity->flipX)
                    entity->velX.whole = Entity_AdjustSpeedForSlope(entity, 0x8000);
                else
                    entity->velX.whole = Entity_AdjustSpeedForSlope(entity, -0x8000);
                if ((anim = entity->animStep) == 1
                    && Target_IsInRect(entity->posX.part16.integer - 0x32, entity->posY.part16.integer - 0x40, 0x64, 0x46)
                    && (RandomNumberGenerator() & 0x7F) <= 0x5F)
                {
                    entity->state = anim;
                    entity->subState = 0;
                    entity->timer = 0;
                    Anim_StartIndexed(entity, 1, 0, 0);
                    break;
                }
            }
        }
        else if (!entity->animTimer)
        {
            entity->velX.whole = 0;
        }
        if (*(u16 *)&entity->animStep == 4)
            PlaySong(0x85);
        Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 8);
        break;
    }
}

/**
 * @brief 93460 | Enemy attack sub-state (Zombie / Medusa family, file
 * 0x08093218): for 8 frames it lunges at 0x10000 / frame in its facing
 * (Entity_AdjustSpeedForSlope), then stops; moves via Entity_MoveWithCollision; at the end of the
 * animation it returns to state 0 walking at 0x8000; otherwise song 0x85
 * on frames 4, 7, 10 and 13.
 */
void Curly_UpdateAttack(struct EwramData_EntityData *entity)
{
    if (entity->subState != 0)
        return;
    if (entity->timer <= 7)
    {
        entity->timer++;
        if (entity->flipX)
            entity->velX.whole = Entity_AdjustSpeedForSlope(entity, 0x10000);
        else
            entity->velX.whole = Entity_AdjustSpeedForSlope(entity, -0x10000);
        entity->velY.whole = 0x10000;
    }
    else
        entity->velX.whole = 0;
    entity->accelY.whole = 0x2800;
    Entity_MoveWithCollision(entity, 0x10000, -0x200000, 0x80000, 8);
    if (entity->animDone)
    {
        Anim_StartIndexed(entity, 0, 0, 1);
        entity->state = 0;
        entity->subState = 0;
        if (entity->flipX)
            entity->velX.whole = Entity_AdjustSpeedForSlope(entity, 0x8000);
        else
            entity->velX.whole = Entity_AdjustSpeedForSlope(entity, -0x8000);
        entity->velY.whole = 0x10000;
        entity->accelY.whole = 0x2800;
        return;
    }
    if (entity->animTimer == 0
        && (entity->animStep == 4 || entity->animStep == 7 || entity->animStep == 10 || entity->animStep == 13))
        PlaySong(0x85);
}

u8 Curly_LoadGraphics(struct EwramData_EntityData *entity)
{
    s32 slot = ObjGfx_LoadResource(sCurlyGfx, 0);
    u16 x, y;
    s32 room;
    if (slot < 0) return 0;
    if (entity->enemyId == 0x2B)
        entity->basePalette = ObjPalette_FindOrLoadVariant(sCurlyPalette, 1);
    else entity->basePalette = ObjPalette_FindOrLoadVariant(sCurlyPalette, 0);
    Sprite_Attach(entity, sCurlySpriteSheet, slot, entity->basePalette);
    x = GetEntityRoomXPositionInteger(entity);
    y = GetEntityRoomYPositionInteger(entity);
    room = GetRoomFromMapPosition((s16)x, (s16)y);
    if (GetAreaFromMapPosition((s16)x, (s16)y) == 4 && (u32)(room - 12) <= 1)
        entity->drawFunc = Entity_DrawWithOffsetCopy;
    else entity->drawFunc = Sprite_DrawEntity;
    return 1;
}

void Curly_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Player_CalculateAttackPower(hit[2]);
    damage = Hit_ResolveDamage(entity, (s32)attacker, hit, (s16)damage);
    entity->hitFlashTimer = 8;
    {
        u16 hp = entity->hp;
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, (s16)damage, 0);
    if ((s16)entity->hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x1C3);
        Anim_StartIndexed(entity, 2, 0, 0);
        entity->palette = entity->basePalette;
        entity->timer = 0x28;
        entity->state = 2;
        entity->subState = 0;
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Curly_DamagePlayerOnContact = Enemy_DamagePlayerOnContact */

/**
 * @brief 93674 | Same as Enemy_DamagePlayerOnContact.
 */
void Curly_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   EnemyCurlyUpdate = EnemySkeletonKnightUpdate */

/**
 * @brief 936A0 | Same as EnemySkeletonKnightUpdate.
 */
void EnemyCurlyUpdate(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (Enemy_PreUpdate(entity))
        return;
    switch (entity->state)
    {
        case 0:
            Curly_UpdateWalk(entity);
            break;
        case 1:
            Curly_UpdateAttack(entity);
            break;
        case 2:
            Enemy_UpdateFragmentDeath(entity);
            break;
        case 3:
            Anim_Update(entity);
            return;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}

/* Forward this enemy variant to its shared implementation. */
void EnemyDurgaUpdate(struct EwramData_EntityData *entity)
{
    EnemyCurlyUpdate(entity);
}

/* Forward this enemy variant to its shared implementation. */
void EnemyDurgaCreate(struct EwramData_EntityData *entity)
{
    EnemyCurlyCreate(entity);
}
