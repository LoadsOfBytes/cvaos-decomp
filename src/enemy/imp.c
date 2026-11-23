/*
 * Imp and Tiny Devil.
 * Built without -f2003-patch (the Tiny Devil animation / hit file needs it off; the rest compiles the same either way).
 */
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "syscalls.h"

/* Tiny Devil update and states from 0x080C47F8. */
extern void Imp_UpdateState4Animation(struct EwramData_EntityData *entity);
extern s32 Enemy_IsStunned(struct EwramData_EntityData *entity);
extern void Hit_CheckWithPlayer(void *res, struct EwramData_EntityData *b);
extern s32 Target_GetX(struct EwramData_EntityData *entity);
extern s32 Target_GetY(struct EwramData_EntityData *entity);
extern void Entity_AnimateWithBoxes(struct EwramData_EntityData *entity);
extern void Imp_UpdateFlight(struct EwramData_EntityData *entity);
extern void Imp_UpdateAttack(struct EwramData_EntityData *entity);
extern void Imp_UpdateGrab(struct EwramData_EntityData *entity);
extern void Imp_UpdateDeath(struct EwramData_EntityData *entity);

/**
 * @brief C47F8 | Tiny Devil update: state 4 only runs Imp_UpdateState4Animation.
 * Otherwise Player_TrackEntity, then collision (Hit_CheckWithPlayer) and done when
 * Enemy_IsStunned holds outside states 3 and 0xFF. State 2 at the target
 * position becomes state 1; run the state (0/1 Imp_UpdateFlight, 2
 * Imp_UpdateAttack, 3 Imp_UpdateGrab, 0xFF Imp_UpdateDeath), then Entity_AnimateWithBoxes and
 * collision unless the state became 0xFF or 3.
 */
void EnemyImpUpdate(struct EwramData_EntityData *entity)
{
    u32 collision[4];

    if (entity->state == 4)
    {
        Imp_UpdateState4Animation(entity);
        return;
    }
    Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity) && entity->state != 0xFF && entity->state != 3)
    {
        Hit_CheckWithPlayer(collision, entity);
        return;
    }
    if (Target_GetX(entity) == entity->posX.whole
        && Target_GetY(entity) == entity->posY.whole
        && entity->state == 2)
    {
        entity->state = 1;
        entity->stage = 0;
        entity->subState = 0;
    }
    switch (entity->state)
    {
        case 0:
        case 1:
            Imp_UpdateFlight(entity);
            break;
        case 2:
            Imp_UpdateAttack(entity);
            break;
        case 3:
            Imp_UpdateGrab(entity);
            break;
        case 0xFF:
            Imp_UpdateDeath(entity);
            break;
    }
    if (entity->state == 0xFF)
        return;
    Entity_AnimateWithBoxes(entity);
    if (entity->state == 0xFF || entity->state == 3)
        return;
    Hit_CheckWithPlayer(collision, entity);
}

extern s32 Entity_GetTargetDistanceX(struct EwramData_EntityData *entity);
extern s32 Entity_GetTargetDistanceY(struct EwramData_EntityData *entity);
extern void Entity_FaceTarget(struct EwramData_EntityData *entity);
extern s32 EntityIsOutsideScreen(struct EwramData_EntityData *entity, s32 margin);

#define ABS(x) ((x) < 0 ? -(x) : (x))

/**
 * @brief Approach and brake near the target, bob vertically and enter attack state 2 when the flight countdown expires on screen.
 */
void Imp_UpdateFlight(struct EwramData_EntityData *entity)
{
    s32 dx = (s16)Entity_GetTargetDistanceX(entity);
    s32 dy = (s16)Entity_GetTargetDistanceY(entity);

    if (Target_GetX(entity) == entity->posX.whole && Target_GetY(entity) == entity->posY.whole)
    {
        switch (entity->stage)
        {
            case 0:
                entity->velX.whole = 0;
                entity->accelX.whole = 0;
                entity->velY.whole = 0;
                entity->accelY.whole = 0;
                entity->stage++;
            case 1:
                entity->velY.whole = Sin((u16)entity->ext.imp.bobPhase + 0x4000) / 4;
                entity->ext.imp.bobPhase += 0x200;
                entity->posY.whole += entity->velY.whole;
                entity->velY.whole += entity->accelY.whole;
                return;
        }
    }
    else
    {
        entity->stage = 0;
    }
    if (entity->ext.imp.attackTimer != 0)
        entity->ext.imp.attackTimer--;
    Entity_FaceTarget(entity);
    switch (entity->subState)
    {
        case 0:
            entity->ext.imp.attackTimer = RandomNumberGenerator() % 360 + 100;
            entity->ext.imp.braking = 0;
            entity->subState++;
            entity->ext.imp.inPositionX = 0;
            entity->ext.imp.inPositionY = 0;
            entity->velX.whole = 0;
            entity->accelY.whole = 0;
            entity->velY.whole = 0;
            entity->accelX.whole = 0x1000;
            if (dx > 70)
                entity->accelX.whole = -entity->accelX.whole;
            entity->accelY.whole = 0x1000;
            if (dy > -45)
                entity->accelY.whole = -entity->accelY.whole;
        case 1:
            if (entity->ext.imp.braking & 1)
            {
                s32 hi = ABS(entity->accelX.whole);
                s32 lo = -ABS(entity->accelX.whole);

                if ((entity->velX.whole >= lo && entity->velX.whole <= hi) || entity->velX.whole == 0)
                {
                    entity->ext.imp.braking &= ~1;
                    entity->velX.whole = 0;
                    entity->accelX.whole = 0;
                    entity->ext.imp.inPositionX = 1;
                }
                else
                {
                    entity->ext.imp.inPositionX = 0;
                }
            }
            else if (dx > 70 || dx < -70)
            {
                if (entity->velX.whole > 0x20000)
                    entity->velX.whole = 0x20000;
                if (entity->velX.whole < -0x20000)
                    entity->velX.whole = -0x20000;
                entity->accelX.whole = 0x1000;
                if (dx > 70)
                    entity->accelX.whole = -entity->accelX.whole;
            }
            else if (entity->velX.whole != 0 && entity->accelX.whole != 0)
            {
                entity->ext.imp.braking |= 1;
                entity->accelX.whole = -entity->accelX.whole;
            }
            else
            {
                entity->velX.whole = 0;
                entity->accelX.whole = 0;
                entity->ext.imp.inPositionX = 1;
            }

            if (entity->ext.imp.braking & 2)
            {
                s32 hi = ABS(entity->accelY.whole);
                s32 lo = -ABS(entity->accelY.whole);

                if ((entity->velY.whole >= lo && entity->velY.whole <= hi) || entity->velY.whole == 0)
                {
                    entity->ext.imp.braking &= ~2;
                    entity->velY.whole = 0;
                    entity->accelY.whole = 0;
                    entity->ext.imp.inPositionY = 1;
                }
                else
                {
                    entity->ext.imp.inPositionY = 0;
                }
            }
            else if (dy > -60 || dy < -80)
            {
                if (entity->velY.whole > 0x14000)
                    entity->velY.whole = 0x14000;
                if (entity->velY.whole < -0x14000)
                    entity->velY.whole = -0x14000;
                entity->accelY.whole = 0x1000;
                if (dy > -60)
                    entity->accelY.whole = -entity->accelY.whole;
            }
            else if (entity->velY.whole != 0 && entity->accelY.whole != 0)
            {
                entity->ext.imp.braking |= 2;
                entity->accelY.whole = -entity->accelY.whole;
            }
            else
            {
                entity->velY.whole = 0;
                entity->accelY.whole = 0;
                entity->ext.imp.inPositionY = 1;
            }

            if (entity->ext.imp.inPositionX && entity->ext.imp.inPositionY)
            {
                entity->velX.whole = 0;
                entity->accelX.whole = 0;
                entity->velY.whole = 0;
                entity->accelY.whole = 0;
                entity->subState = 2;
            }
            break;
        case 2:
            entity->velY.whole = Sin((u16)entity->ext.imp.bobPhase + 0x4000) / 4;
            entity->ext.imp.bobPhase += 0x200;
            if (dx > 70 || dx < -70 || dy < -80 || dy > 0)
            {
                entity->subState = 1;
                entity->ext.imp.inPositionX = 0;
                entity->ext.imp.inPositionY = 0;
            }
            if (entity->ext.imp.attackTimer == 0 && !EntityIsOutsideScreen(entity, 4))
            {
                entity->velX.whole = 0;
                entity->accelX.whole = 0;
                entity->velY.whole = 0;
                entity->accelY.whole = 0;
                entity->state = 2;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
}

extern s32 Entity_AimAccelerationAtPoint(struct EwramData_EntityData *entity, s32 targetX, s32 targetY, s32 speed);

/**
 * @brief Spring sideways, accelerate toward the target offset, then brake both axes and advance to grab state 3.
 */
void Imp_UpdateAttack(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 targetX = Target_GetX(entity) + bg->xPos.whole;
    s32 targetY = Target_GetY(entity) + bg->yPos.whole - 0x200000;
    s32 dx = (s16)Entity_GetTargetDistanceX(entity);

    Entity_GetTargetDistanceY(entity);
    switch (entity->subState)
    {
        case 0:
            entity->velY.whole = 0;
            entity->accelY.whole = 0;
            entity->velX.whole = 0x10000;
            entity->accelX.whole = 0x800;
            if (dx > 0)
                entity->accelX.whole = -entity->accelX.whole;
            else
                entity->velX.whole = -entity->velX.whole;
            entity->velY.whole = 0x10000;
            entity->accelY.whole = -0x800;
            entity->timer = 0;
            entity->subState = 1;
        case 1:
            if (entity->timer++ > 30)
            {
                entity->velX.whole = 0;
                entity->accelX.whole = 0;
                entity->velY.whole = 0;
                entity->accelY.whole = 0;
                entity->timer = 0;
                entity->subState = 2;
                Entity_AimAccelerationAtPoint(entity, targetX, targetY, 0x800);
            }
            break;
        case 2:
            if (entity->timer++ > 80)
            {
                if (entity->accelX.whole > 0)
                    entity->accelX.whole = -0x800;
                else
                    entity->accelX.whole = 0x800;
                if (entity->accelY.whole > 0)
                    entity->accelY.whole = -0x800;
                else
                    entity->accelY.whole = 0x800;
                entity->timer = 0;
                entity->subState++;
            }
            break;
        case 3:
            if (entity->accelX.whole < 0 && entity->velX.whole <= 0)
            {
                entity->velX.whole = 0;
                entity->accelX.whole = 0;
            }
            if (entity->accelX.whole > 0 && entity->velX.whole >= 0)
            {
                entity->velX.whole = 0;
                entity->accelX.whole = 0;
            }
            if (entity->accelY.whole < 0 && entity->velY.whole <= 0)
            {
                entity->velY.whole = 0;
                entity->accelY.whole = 0;
            }
            if (entity->accelY.whole > 0 && entity->velY.whole >= 0)
            {
                entity->velY.whole = 0;
                entity->accelY.whole = 0;
            }
            if (entity->velX.whole == 0 && entity->velY.whole == 0)
                entity->state++;
            break;
        case 4:
            if (entity->timer++ > 30)
            {
                entity->timer = 0;
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
            break;
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
}

extern s32 Entity_AimVelocityAtPointByDistance(struct EwramData_EntityData *entity, s32 targetX, s32 targetY, s32 speed);

/* Same as CheckUnk_53C_6 in player/player_core.c. */
static inline s32 IsNotFlippedX(struct EwramData_EntityData *entity)
{
    return entity->flipX == 0;
}

/**
 * @brief Follow an offset from the target, alternate forced player input, and detach after enough qualifying new-input events; then return to flight.
 */
void Imp_UpdateGrab(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 targetX = Target_GetX(entity) + bg->xPos.whole;
    s32 targetY = Target_GetY(entity) + bg->yPos.whole - 0x200000;
    struct InputData *input = &gEwramData->inputData;
    struct EwramData_EntityData *other = &gEwramData->entityData[entity->ext.imp.linkIndex];

    Entity_FaceTarget(entity);
    switch (entity->subState)
    {
        case 0:
            other->state++;
            other->stage = 0;
            other->subState = 0;
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = 0;
            entity->accelY.whole = 0;
            entity->timer = 0;
            entity->ext.imp.grabTimer = 0;
            entity->subState = 1;
        case 1:
            if (IsNotFlippedX(gEwramData->playerState.player))
                targetX -= 0x100000;
            else
                targetX += 0x100000;
            if (!Entity_AimVelocityAtPointByDistance(entity, targetX, targetY, 0x10000))
            {
                entity->posX.whole += entity->velX.whole;
                entity->posY.whole += entity->velY.whole;
                entity->velX.whole += entity->accelX.whole;
                entity->velY.whole += entity->accelY.whole;
            }
            if (entity->timer++ & 1)
                SetPlayerInput(1, 2);
            else
                SetPlayerInput(1, 0);
            if (input->newInput != 0 && input->newInput != 4 && input->newInput != 8
                && (u8)entity->ext.imp.grabTimer++ > 19)
            {
                if (entity->velX.whole > 0)
                    entity->velX.whole = -0x10000;
                else
                    entity->velX.whole = 0x10000;
                entity->velY.whole = -0x10000;
                other->state++;
                other->stage = 0;
                other->subState = 0;
                entity->timer = 0;
                entity->subState = 2;
            }
            break;
        case 2:
            entity->posX.whole += entity->velX.whole;
            entity->posY.whole += entity->velY.whole;
            if (entity->timer++ > 39)
                entity->subState = 3;
            break;
        case 3:
            entity->timer = 0;
            entity->velX.whole = 0;
            entity->accelX.whole = 0;
            entity->velY.whole = 0;
            entity->accelY.whole = 0;
            entity->state = 1;
            entity->stage = 0;
            entity->subState = 0;
            break;
    }
}

extern void PlaySong(u16 song);
extern void Enemy_DropRewards(struct EwramData_EntityData *entity);
extern s32 Imp_SpawnDeathPiece(struct EwramData_EntityData *entity);
extern void Enemy_UpdateFragmentDeathWithSpread(struct EwramData_EntityData *entity, s32 interval, s32 width, s32 height);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *entity);

/**
 * @brief Drop rewards, delete the grab indicator, spawn a falling piece and fall while running the fragment-death effect.
 */
void Imp_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *other = &gEwramData->entityData[entity->ext.imp.linkIndex];

    switch (entity->subState)
    {
        case 0:
            PlaySong(0x72);
            Enemy_DropRewards(entity);
            entity->palette = entity->basePalette;
            entity->timer = 50;
            entity->state = 0xFF;
            entity->subState++;
            entity->animFrame = 20;
            other->deleteRequested = 1;
            entity->velY.whole = 0x8000;
            Imp_SpawnDeathPiece(entity);
        case 1:
            entity->posY.whole += entity->velY.whole;
            if (entity->posY.part16.integer > 170)
                entity->deleteRequested = 1;
            break;
    }
    Enemy_UpdateFragmentDeathWithSpread(entity, 7, 0x100000, 0xC0000);
}

/**
 * @brief During attack, enter grab state when the target/player coordinate test and time-stop test allow it; otherwise submit the contact record outside grab state.
 */
void Imp_OnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;
    s32 decoy = 0;
    struct EwramData_EntityData *target;
    struct EwramData_EntityData *player;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = decoy;
    contactDamage.unused = decoy;
    contactDamage.reaction = decoy;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    target = gEwramData->playerState.enemyTarget;
    player = gEwramData->playerState.player;
    if (target->posX.whole != player->posX.whole)
        decoy = target->posY.whole != player->posY.whole;
    if (entity->state == 3)
        return;
    if (entity->state == 2 && !Entity_IsTimeStopped(entity) && !decoy)
    {
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
        return;
    }
    Player_ApplyContactDamage(&contactDamage);
}

extern s32 ObjPalette_FindOrLoad(void *palette);
extern s32 Entity_GetVisibilityPattern(s32 step);

/* Graphics of the Tiny Devil (also used by Imp_LoadGraphics). */
extern u8 sImpTinyDevilGfx[];
extern u8 sImpTinyDevilPalette[];
extern u8 sImpTinyDevilSpriteSheet[];

/**
 * @brief Initialize hidden, show animation 3 when activated and follow 40 pixels above the target; hide again when released.
 */
void ImpGrabIndicator_Update(struct EwramData_EntityData *entity)
{
    void *palette = sImpTinyDevilPalette;
    void *sprite = sImpTinyDevilSpriteSheet;
    u8 *graphics = sImpTinyDevilGfx;
    s32 gfx;

    switch (entity->state)
    {
        case 0:
            entity->scrollWithBg = 1;
            gfx = ObjGfx_Find(graphics);
            Sprite_Attach(entity, sprite, gfx, (s8)ObjPalette_FindOrLoad(palette));
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            entity->hidden = 1;
            entity->state++;
            break;
        case 1:
            break;
        case 2:
            entity->hidden = 0;
            Anim_StartIndexed(entity, 3, 0, 1);
            entity->state++;
        case 3:
            entity->posX.whole = Target_GetX(entity);
            entity->posY.whole = Target_GetY(entity) - 0x280000;
            Anim_Update(entity);
            break;
        case 4:
            entity->hidden = 1;
            entity->state = 1;
            break;
    }
}

/**
 * @brief Start below the owner with 1.5 times its vertical velocity, fall, apply the visibility table and delete after the timer or screen-Y limit.
 */
void ImpDeathPiece_Update(struct EwramData_EntityData *entity)
{
    void *palette = sImpTinyDevilPalette;
    void *sprite = sImpTinyDevilSpriteSheet;
    u8 *graphics = sImpTinyDevilGfx;
    struct EwramData_EntityData *other = &gEwramData->entityData[entity->ext.imp.linkIndex];
    s32 gfx;

    switch (entity->state)
    {
        case 0:
            entity->scrollWithBg = 1;
            gfx = ObjGfx_Find(graphics);
            Sprite_Attach(entity, sprite, gfx, (s8)ObjPalette_FindOrLoad(palette));
            entity->drawFunc = (u32 *)Sprite_DrawEntity;
            entity->animFrame = 0x15;
            entity->spriteLayer = 3;
            entity->posX.whole = other->posX.whole;
            entity->posY.whole = other->posY.whole + 0x40000;
            entity->velY.whole = other->velY.whole * 3 / 2;
            entity->timer = 0;
            entity->state++;
            break;
        case 1:
            entity->posY.whole += entity->velY.whole;
            if (entity->timer++ > 50 || entity->posY.part16.integer > 170)
                entity->state = 2;
            break;
        case 2:
            entity->deleteRequested = 1;
            break;
    }
    entity->hidden = Entity_GetVisibilityPattern(entity->timer);
}

/* Tiny Devil helpers 0x080C5200..0x080C52F3 (same template as the Ukoback
 * helpers in enemy/ukoback.c). */

extern u16 Player_CalculateAttackPower(u16 attackCategory);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *entity, s32 other, u16 *hit, s32 inputDamage);
extern void Entity_ShowDamage(struct EwramData_EntityData *owner, s16 value, s32 kind);

/**
 * @brief Clear velocity and acceleration, face the target and reset state 1 with zero stage and substate.
 */
void Imp_ResetFlight(struct EwramData_EntityData *entity)
{
    s32 zero = 0;

    entity->velX.whole = zero;
    entity->velY.whole = zero;
    entity->accelX.whole = zero;
    entity->accelY.whole = zero;
    Entity_FaceTarget(entity);
    entity->state = 1;
    entity->stage = zero;
    entity->subState = zero;
}

/**
 * @brief Start animation 0 once and then advance it. The gameplay purpose of state 4 remains unresolved.
 */
void Imp_UpdateState4Animation(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            Anim_StartIndexed(entity, 0, 0, 1);
            entity->subState++;
        case 1:
            Anim_Update(entity);
            break;
    }
}

/**
 * @brief Resolve damage, subtract health, show damage and flash; start the death sequence on lethal damage.
 */
void Imp_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));

    {
        u16 hp = entity->hp;
        damage = (damage << 16) >> 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    entity->hitFlashTimer = 8;
    if ((s16)entity->hp <= 0)
    {
        entity->subState = 0;
        Imp_UpdateDeath(entity);
    }
}

/**
 * @brief Create the indicator callback in effect slots and store its slot in the parent; return 1 on success, otherwise 0.
 */
s32 Imp_CreateGrabIndicator(struct EwramData_EntityData *entity)
{
    s32 created = 0;
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)ImpGrabIndicator_Update);

    if (child != NULL)
    {
        entity->ext.imp.linkIndex = ENTITY_INDEX(child);
        created = 1;
    }
    return created;
}

/**
 * @brief Create a falling death piece with the parent slot in its owner field; return 1 on success, otherwise 0.
 */
s32 Imp_SpawnDeathPiece(struct EwramData_EntityData *entity)
{
    s32 created = 0;
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)ImpDeathPiece_Update);

    if (child != NULL)
    {
        child->ext.imp.linkIndex = ENTITY_INDEX_PAIR(entity);
        created = 1;
    }
    return created;
}

/* Tiny Devil create and update from 0x080C52F4 (same graphics as the
 * enemy in this file). */
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *resource, void *palette, u8 variant, void *sheet);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *entity);
extern void Entity_EnableBoxes(struct EwramData_EntityData *entity, void *onHit, s32 onHurt);
extern void Enemy_OnContact(struct EwramData_EntityData *entity);
extern void TinyDevil_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void TinyDevil_UpdateState3Animation(struct EwramData_EntityData *entity);
extern void TinyDevil_UpdateDeath(struct EwramData_EntityData *entity);
extern void TinyDevil_UpdateFlight(struct EwramData_EntityData *entity);
extern void TinyDevil_UpdateAttack(struct EwramData_EntityData *entity);
extern s32 Entity_IsInRect(struct EwramData_EntityData *entity, s16 left, s16 top, u16 width, u16 height);
extern void Enemy_CheckHitPlayer(struct EwramData_EntityData *entity);

/* Graphics shared with the enemy in this file. */

/**
 * @brief C52F4 | Tiny Devil create: load the graphics (delete the enemy
 * when that fails), sprite 1, hit boxes, layer 4, state 1; state 3
 * instead when bestiaryPreview is set.
 */
void EnemyTinyDevilCreate(struct EwramData_EntityData *entity)
{
    void *palette = sImpTinyDevilPalette;
    void *sprite = sImpTinyDevilSpriteSheet;
    u8 *graphics = sImpTinyDevilGfx;

    if (!Entity_LoadGraphics(entity, graphics, palette, 1, sprite))
    {
        EntityDelete(entity);
        return;
    }
    Anim_StartIndexed(entity, 1, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 4;
    entity->state = 1;
    entity->stage = 0;
    entity->subState = 0;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 3;
        return;
    }
    Entity_EnableBoxes(entity, Enemy_OnContact, (s32)TinyDevil_OnHit);
}

/**
 * @brief C5384 | Tiny Devil update: state 3 TinyDevil_UpdateState3Animation, 0xFF
 * TinyDevil_UpdateDeath. Otherwise Player_TrackEntity on the screen, collision and done
 * when Enemy_IsStunned holds; state 2 at the target position becomes state 1;
 * run the state (1 TinyDevil_UpdateFlight, 2 TinyDevil_UpdateAttack), Entity_AnimateWithBoxes and
 * Enemy_CheckHitPlayer.
 */
void EnemyTinyDevilUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[16];
    s32 stunned;

    if (entity->state == 3)
    {
        TinyDevil_UpdateState3Animation(entity);
        return;
    }
    if (entity->state == 0xFF)
    {
        TinyDevil_UpdateDeath(entity);
        return;
    }
    if (Entity_IsInRect(entity, -16, 0, 272, 176))
        Player_TrackEntity(entity);
    stunned = Enemy_IsStunned(entity);
    if (stunned)
    {
        Hit_CheckWithPlayer(collision, entity);
        return;
    }
    if (Target_GetX(entity) == entity->posX.whole && Target_GetY(entity) == entity->posY.whole
        && entity->state == 2)
    {
        entity->state = 1;
        entity->stage = stunned;
        entity->subState = stunned;
    }
    switch (entity->state)
    {
        case 1:
            TinyDevil_UpdateFlight(entity);
            break;
        case 2:
            TinyDevil_UpdateAttack(entity);
            break;
    }
    Entity_AnimateWithBoxes(entity);
    Enemy_CheckHitPlayer(entity);
}

/**
 * @brief Oscillate around the target using randomized axis turning points; enter attack state 2 when the flight countdown expires on screen.
 */
void TinyDevil_UpdateFlight(struct EwramData_EntityData *entity)
{
    s32 dx = (s16)Entity_GetTargetDistanceX(entity);
    s32 dy = (s16)Entity_GetTargetDistanceY(entity);

    if (Target_GetX(entity) == entity->posX.whole && Target_GetY(entity) == entity->posY.whole)
    {
        switch (entity->stage)
        {
            case 0:
                entity->velX.whole = 0;
                entity->accelX.whole = 0;
                entity->velY.whole = 0;
                entity->accelY.whole = 0;
                entity->stage++;
            case 1:
                entity->velY.whole = Sin((u16)entity->ext.tinyDevil.bobPhase + 0x4000) / 4;
                entity->ext.tinyDevil.bobPhase += 0x200;
                entity->posY.whole += entity->velY.whole;
                entity->velY.whole += entity->accelY.whole;
                return;
        }
    }
    else
    {
        entity->stage = 0;
    }
    if ((s16)entity->ext.tinyDevil.attackTimer != 0)
        entity->ext.tinyDevil.attackTimer--;
    Entity_FaceTarget(entity);
    switch (entity->subState)
    {
        case 0:
            if ((s16)entity->ext.tinyDevil.attackTimer == 0)
                entity->ext.tinyDevil.attackTimer = RandomNumberGenerator() % 500 + 100;
            entity->ext.tinyDevil.flags = 0;
            if (entity->animScriptIndex != 1)
                Anim_StartIndexed(entity, 1, 0, 1);
            entity->accelX.whole = 0x200;
            entity->accelY.whole = 0x400;
            if (dx > 0)
            {
                entity->ext.tinyDevil.offsetX = RandomNumberGenerator() % 25 - 25;
                entity->accelX.whole = -entity->accelX.whole;
                entity->ext.tinyDevil.flags &= ~4;
            }
            else
            {
                entity->ext.tinyDevil.offsetX = 25 - RandomNumberGenerator() % 25;
                entity->ext.tinyDevil.flags |= 4;
            }
            if (dy > 25)
            {
                entity->ext.tinyDevil.offsetY = RandomNumberGenerator() % 25 - 20;
                entity->accelY.whole = -entity->accelY.whole;
                entity->ext.tinyDevil.flags &= ~8;
            }
            else
            {
                entity->ext.tinyDevil.offsetY = -70 - RandomNumberGenerator() % 25;
                entity->ext.tinyDevil.flags |= 8;
            }
            entity->subState++;
            break;
        case 1:
            if (entity->ext.tinyDevil.flags & 4)
            {
                if (entity->ext.tinyDevil.flags & 1)
                {
                    if (entity->velX.whole <= 0)
                    {
                        entity->ext.tinyDevil.flags &= ~1;
                        entity->subState = 0;
                    }
                }
                else
                {
                    if (entity->velX.whole > 0x18000)
                        entity->velX.whole = 0x18000;
                    if (dx >= entity->ext.tinyDevil.offsetX)
                    {
                        entity->ext.tinyDevil.flags |= 1;
                        if (entity->velX.whole > 0x10000)
                            entity->accelX.whole = -(entity->accelX.whole * 3);
                        else
                            entity->accelX.whole = -entity->accelX.whole;
                    }
                }
            }
            else
            {
                if (entity->ext.tinyDevil.flags & 1)
                {
                    if (entity->velX.whole >= 0)
                    {
                        entity->ext.tinyDevil.flags &= ~1;
                        entity->subState = 0;
                    }
                }
                else
                {
                    if (entity->velX.whole < -0x18000)
                        entity->velX.whole = -0x18000;
                    if (dx <= entity->ext.tinyDevil.offsetX)
                    {
                        entity->ext.tinyDevil.flags |= 1;
                        if (entity->velX.whole < -0x10000)
                            entity->accelX.whole = -(entity->accelX.whole * 3);
                        else
                            entity->accelX.whole = -entity->accelX.whole;
                    }
                }
            }

            if (entity->ext.tinyDevil.flags & 8)
            {
                if (entity->ext.tinyDevil.flags & 2)
                {
                    if (entity->velY.whole <= 0)
                    {
                        entity->ext.tinyDevil.flags &= ~2;
                        entity->subState = 0;
                    }
                }
                else
                {
                    if (entity->velY.whole > 0x8000)
                        entity->velY.whole = 0x8000;
                    if (dy >= entity->ext.tinyDevil.offsetY)
                    {
                        entity->ext.tinyDevil.flags |= 2;
                        if (entity->velY.whole > 0x10000)
                            entity->accelY.whole = -(entity->accelY.whole * 3);
                        else
                            entity->accelY.whole = -entity->accelY.whole;
                    }
                }
            }
            else
            {
                if (entity->ext.tinyDevil.flags & 2)
                {
                    if (entity->velY.whole >= 0)
                    {
                        entity->ext.tinyDevil.flags &= ~2;
                        entity->subState = 0;
                    }
                }
                else
                {
                    if (entity->velY.whole < -0x8000)
                        entity->velY.whole = -0x8000;
                    if (dy <= entity->ext.tinyDevil.offsetY)
                    {
                        entity->ext.tinyDevil.flags |= 2;
                        if (entity->velY.whole < -0x10000)
                            entity->accelY.whole = -(entity->accelY.whole * 3);
                        else
                            entity->accelY.whole = -entity->accelY.whole;
                    }
                }
            }
            break;
    }
    if ((s16)entity->ext.tinyDevil.attackTimer == 0 && !EntityIsOutsideScreen(entity, 4))
    {
        entity->velX.whole = 0;
        entity->accelX.whole = 0;
        entity->velY.whole = 0;
        entity->accelY.whole = 0;
        entity->state = 2;
        entity->stage = 0;
        entity->subState = 0;
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
}

extern s32 Entity_IsNearScreenEdge(struct EwramData_EntityData *entity, s32 margin);
extern struct EwramData_EntityData *TinyDevil_SpawnProjectile(struct EwramData_EntityData *entity);

/**
 * @brief Spring sideways and upward, abort near a screen edge or create a projectile after the delay, then return to flight.
 */
void TinyDevil_UpdateAttack(struct EwramData_EntityData *entity)
{
    s32 dx;
    s32 atEdge;

    Target_GetX(entity);
    Target_GetY(entity);
    dx = (s16)Entity_GetTargetDistanceX(entity);
    Entity_GetTargetDistanceY(entity);
    switch (entity->subState)
    {
        case 0:
            entity->velY.whole = 0;
            entity->accelY.whole = 0;
            entity->velX.whole = 0x10000;
            entity->accelX.whole = 0x800;
            if (dx > 0)
                entity->accelX.whole = -entity->accelX.whole;
            else
                entity->velX.whole = -entity->velX.whole;
            entity->velY.whole = -0x10000;
            entity->accelY.whole = 0x800;
            entity->timer = 0;
            entity->subState = 1;
        case 1:
            Entity_FaceTarget(entity);
            atEdge = Entity_IsNearScreenEdge(entity, 8);
            if (atEdge)
            {
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
                break;
            }
            if (entity->timer++ > 30)
            {
                if (entity->animScriptIndex != 2)
                    Anim_StartIndexed(entity, 2, 0, 1);
                TinyDevil_SpawnProjectile(entity);
                entity->velX.whole = atEdge;
                entity->accelX.whole = atEdge;
                entity->velY.whole = atEdge;
                entity->accelY.whole = atEdge;
                entity->timer = atEdge;
                entity->subState = 2;
            }
            break;
        case 2:
            if (entity->timer++ > 80)
            {
                entity->timer = 0;
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
            if (entity->timer == 40 && entity->animScriptIndex != 1)
                Anim_StartIndexed(entity, 1, 0, 1);
            break;
    }
    entity->posX.whole += entity->velX.whole;
    entity->posY.whole += entity->velY.whole;
    entity->velX.whole += entity->accelX.whole;
    entity->velY.whole += entity->accelY.whole;
}

/**
 * @brief Drop rewards, restore the palette and fall while running the fragment-death effect; request deletion below Y 170.
 */
void TinyDevil_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            PlaySong(0x72);
            Enemy_DropRewards(entity);
            entity->palette = entity->basePalette;
            entity->timer = 50;
            entity->state = 0xFF;
            entity->subState++;
            entity->animFrame = 20;
            entity->velY.whole = 0x8000;
        case 1:
            entity->posY.whole += entity->velY.whole;
            if (entity->posY.part16.integer > 170)
                entity->deleteRequested = 1;
            break;
    }
    Enemy_UpdateFragmentDeathWithSpread(entity, 7, 0x100000, 0xC0000);
}

extern s32 Hit_Enable(struct EwramData_EntityData *entity, s32 type, void *onHit, s32 onHurt);
extern s32 HitBox_SetAttackRaw(struct EwramData_EntityData *entity, s32 box);
extern s32 Enemy_PreUpdate(struct EwramData_EntityData *entity);
extern void TinyDevilProjectile_Update(struct EwramData_EntityData *entity);
extern void TinyDevilProjectile_OnContact(struct EwramData_EntityData *entity);

/* Integer part of a - b (16.16), rounded towards zero. */
#define PIXEL_DIFF(a, b) ((a) - (b) < 0 ? -(((b) - (a)) >> 16) : ((a) - (b)) >> 16)

/**
 * @brief Create the projectile, aim toward the target offset, enable its attack box and allocate an affine slot; return NULL after cleanup on failure.
 */
struct EwramData_EntityData *TinyDevil_SpawnProjectile(struct EwramData_EntityData *entity)
{
    void *palette = sImpTinyDevilPalette;
    void *sprite = sImpTinyDevilSpriteSheet;
    u8 *graphics = sImpTinyDevilGfx;
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, (u32 *)TinyDevilProjectile_Update);
    s32 targetX;
    s32 targetY;
    s32 slot;
    u16 dx;
    u16 dy;
    struct BgLayerState *bg;

    if (child == NULL)
        return NULL;
    if (!Entity_LoadGraphics(child, graphics, palette, 1, sprite))
        goto fail;
    child->scrollWithBg = 1;
    child->spriteLayer = entity->spriteLayer - 1;
    child->attack = entity->attack;
    child->posX.whole = entity->posX.whole;
    child->posY.whole = entity->posY.whole + 0x80000;
    if ((s16)Entity_GetTargetDistanceX(child) < 0)
        child->posX.whole += 0x100000;
    else
        child->posX.whole -= 0x100000;
    targetX = Target_GetX(child) + (&gEwramData->bgInfo[1])->xPos.whole;
    targetY = Target_GetY(child) + (&gEwramData->bgInfo[1])->yPos.whole - 0x200000;
    Entity_AimVelocityAtPointByDistance(child, targetX, targetY, 0x30000);
    Anim_StartIndexed(child, 4, 0, 0);
    Anim_Update(child);
    Hit_Enable(child, 10, TinyDevilProjectile_OnContact, 0);
    HitBox_SetAttackRaw(child, 0x0A0AFBFB);
    slot = ObjAffine_Alloc(1);
    if (slot < 0)
    {
    fail:
        EntityDelete(child);
        return NULL;
    }
    bg = &gEwramData->bgInfo[1];
    targetX -= bg->xPos.whole;
    targetY -= bg->yPos.whole;
    dx = PIXEL_DIFF(targetX, child->posX.whole);
    dy = PIXEL_DIFF(targetY, child->posY.whole);
    child->affineIndex = slot;
    ObjAffine_Set(ArcTan2(dx, dy) + 0x9800, 0x10000, 0x10000, 0, 0, child->affineIndex);
    return child;
}

/**
 * @brief Wait through randomized animation repetitions, then move with affine rotation enabled; free the affine slot and request deletion after the flight timer.
 */
void TinyDevilProjectile_Update(struct EwramData_EntityData *entity)
{
    if (Enemy_PreUpdate(entity))
        return;
    switch (entity->state)
    {
        case 0:
            entity->state = 1;
        case 1:
            if (entity->animDone)
            {
                if ((s16)++entity->ext.tinyDevil.attackTimer > RandomNumberGenerator() % 5 + 1)
                {
                    Anim_StartIndexed(entity, 6, 0, 0);
                    entity->affine = 1;
                    entity->state++;
                    PlaySong(0x89);
                }
                else
                {
                    Anim_StartIndexed(entity, 5, 0, 0);
                }
            }
            break;
        case 2:
            entity->posX.whole += entity->velX.whole;
            entity->posY.whole += entity->velY.whole;
            entity->velX.whole += entity->accelX.whole;
            entity->velY.whole += entity->accelY.whole;
            if (entity->timer++ > 60)
            {
                ObjAffine_Free(entity->affineIndex);
                entity->deleteRequested = 1;
            }
            break;
    }
    Anim_Update(entity);
    Enemy_CheckHitPlayer(entity);
}

/* TinyDevil_UpdateState3Animation: variant of an already decompiled function, generated with
 * a cloning script (constants adjusted where needed), checked byte for
 * byte. */

/**
 * @brief Start animation 1 once, then advance it. The gameplay purpose of state 3 remains unresolved.
 */
void TinyDevil_UpdateState3Animation(struct EwramData_EntityData *entity)
{
    if (entity->subState == 0)
    {
        Anim_StartIndexed(entity, 1, 0, 1);
        entity->subState++;
    }
    Anim_Update(entity);
}

/* Ukoback helpers and create from 0x080C5B40. */
extern s32 Enemy_TakeDamage(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
extern void ObjAffine_Free(s32 index);
extern s32 Ukoback_LoadGraphics(struct EwramData_EntityData *entity);
extern void Ukoback_OnContact(struct EwramData_EntityData *entity);
extern void Ukoback_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);

/**
 * @brief Apply damage and reset to death state 0xFF on lethal damage.
 */
void TinyDevil_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        entity->state = 0xFF;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief Submit the attack contact record, free the affine slot and request projectile deletion.
 */
void TinyDevilProjectile_OnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attack = entity->attack;
    contactDamage.attributes = 0;
    contactDamage.unused = 0;
    contactDamage.reaction = 0;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
    ObjAffine_Free(entity->affineIndex);
    entity->deleteRequested = 1;
}

/**
 * @brief Return 1 outside the inclusive inset rectangle of the 240x160 screen, otherwise 0; the inset is supplied in pixels.
 */
s32 Entity_IsNearScreenEdge(struct EwramData_EntityData *entity, s32 margin)
{
    if ((s16)entity->posX.part16.integer >= margin && (s16)entity->posX.part16.integer <= 240 - margin
        && (s16)entity->posY.part16.integer >= margin && (s16)entity->posY.part16.integer <= 160 - margin)
        return 0;
    return 1;
}

/**
 * @brief C5BDC | Ukoback create: load the graphics (delete the enemy when
 * that fails), hit boxes, layer 4; state 1 when bestiaryPreview is set,
 * otherwise collision callbacks, stopped, Entity_FaceTarget and state 0.
 */
void EnemyUkobackCreate(struct EwramData_EntityData *entity)
{
    if (!Ukoback_LoadGraphics(entity))
    {
        EntityDelete(entity);
        return;
    }
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 4;
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 1;
        entity->stage = 0;
        entity->subState = 0;
        return;
    }
    Entity_EnableBoxes(entity, Ukoback_OnContact, (s32)Ukoback_OnHit);
    entity->accelX.whole = 0;
    entity->velX.whole = 0;
    entity->accelY.whole = 0;
    entity->velY.whole = 0;
    entity->ext.ukoback.fireballCount = 0;
    Entity_FaceTarget(entity);
    entity->state = 0;
    entity->stage = 0;
    entity->subState = 0;
}

/* Graphics loader 0x080C5C58. It only matches without -f2003-patch (see
 * NO_2003_PATCH_SRCS in ../Makefile). */
extern s32 ObjPalette_FindOrLoadVariant(void *palette, s32 variant);

/* Graphics of the Ukoback (also used in enemy/ukoback.c). */
extern u8 sUkobackGfx[];
extern u8 sUkobackPalette[];
extern u8 sUkobackSpriteSheet[];

/**
 * @brief Find or load graphics and palette, attach the sprite and start animation 0; return 0 on failure, 1 on success.
 */
s32 Ukoback_LoadGraphics(struct EwramData_EntityData *entity)
{
    void *palette = sUkobackPalette;
    void *sprite = sUkobackSpriteSheet;
    u8 *graphics = sUkobackGfx;
    s32 gfx = ObjGfx_Find(graphics);
    s8 paletteIndex;

    if (gfx < 0)
    {
        gfx = ObjGfx_LoadResource(graphics, 0);
        if (gfx < 0)
            return 0;
    }
    paletteIndex = ObjPalette_FindOrLoadVariant(palette, 0);
    if (paletteIndex == -1)
        return 0;
    entity->basePalette = paletteIndex;
    if (!Sprite_Attach(entity, sprite, gfx, entity->basePalette))
        return 0;
    if (!Anim_StartIndexed(entity, 0, 0, 1))
        return 0;
    Anim_Update(entity);
    entity->drawFunc = (u32 *)Sprite_DrawEntity;
    return 1;
}
