/* Tiny Devil update and states from 0x080C47F8. */
#include "code_08009A0.h"
#include "imp_entity.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "structs/ewram.h"

/*
 * Imp work slots: bob phase of the hovering, frames until the next attack,
 * braking bits (1 = braking horizontally, 2 = vertically) and the two
 * "in position" flags (x / y) that start the attack when both are set, and
 * the frame counter of the grab. Token-identical expansion.
 */
#define IMP_BOB_PHASE(e) e->unk_500.part16.unk_500
#define IMP_ATTACK_TIMER(e) e->unk_4F8.part16.unk_4FA
#define IMP_BRAKING(e) e->unk_4FC.part16.unk_4FC
#define IMP_IN_POSITION_X(e) e->unk_504.part8.unk_504
#define IMP_IN_POSITION_Y(e) e->unk_504.part8.unk_505
#define IMP_GRAB_TIMER(e) e->unk_504.part8.unk_506

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
                entity->velY.whole = Sin((u16)IMP_BOB_PHASE(entity) + 0x4000) / 4;
                IMP_BOB_PHASE(entity) += 0x200;
                entity->posY.whole += entity->velY.whole;
                entity->velY.whole += entity->accelY.whole;
                return;
        }
    }
    else
    {
        entity->stage = 0;
    }
    if (IMP_ATTACK_TIMER(entity) != 0)
        IMP_ATTACK_TIMER(entity)--;
    Entity_FaceTarget(entity);
    switch (entity->subState)
    {
        case 0:
            IMP_ATTACK_TIMER(entity) = RandomNumberGenerator() % 360 + 100;
            IMP_BRAKING(entity) = 0;
            entity->subState++;
            IMP_IN_POSITION_X(entity) = 0;
            IMP_IN_POSITION_Y(entity) = 0;
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
            if (IMP_BRAKING(entity) & 1)
            {
                s32 hi = ABS(entity->accelX.whole);
                s32 lo = -ABS(entity->accelX.whole);

                if ((entity->velX.whole >= lo && entity->velX.whole <= hi) || entity->velX.whole == 0)
                {
                    IMP_BRAKING(entity) &= ~1;
                    entity->velX.whole = 0;
                    entity->accelX.whole = 0;
                    IMP_IN_POSITION_X(entity) = 1;
                }
                else
                {
                    IMP_IN_POSITION_X(entity) = 0;
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
                IMP_BRAKING(entity) |= 1;
                entity->accelX.whole = -entity->accelX.whole;
            }
            else
            {
                entity->velX.whole = 0;
                entity->accelX.whole = 0;
                IMP_IN_POSITION_X(entity) = 1;
            }

            if (IMP_BRAKING(entity) & 2)
            {
                s32 hi = ABS(entity->accelY.whole);
                s32 lo = -ABS(entity->accelY.whole);

                if ((entity->velY.whole >= lo && entity->velY.whole <= hi) || entity->velY.whole == 0)
                {
                    IMP_BRAKING(entity) &= ~2;
                    entity->velY.whole = 0;
                    entity->accelY.whole = 0;
                    IMP_IN_POSITION_Y(entity) = 1;
                }
                else
                {
                    IMP_IN_POSITION_Y(entity) = 0;
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
                IMP_BRAKING(entity) |= 2;
                entity->accelY.whole = -entity->accelY.whole;
            }
            else
            {
                entity->velY.whole = 0;
                entity->accelY.whole = 0;
                IMP_IN_POSITION_Y(entity) = 1;
            }

            if (IMP_IN_POSITION_X(entity) && IMP_IN_POSITION_Y(entity))
            {
                entity->velX.whole = 0;
                entity->accelX.whole = 0;
                entity->velY.whole = 0;
                entity->accelY.whole = 0;
                entity->subState = 2;
            }
            break;
        case 2:
            entity->velY.whole = Sin((u16)IMP_BOB_PHASE(entity) + 0x4000) / 4;
            IMP_BOB_PHASE(entity) += 0x200;
            if (dx > 70 || dx < -70 || dy < -80 || dy > 0)
            {
                entity->subState = 1;
                IMP_IN_POSITION_X(entity) = 0;
                IMP_IN_POSITION_Y(entity) = 0;
            }
            if (IMP_ATTACK_TIMER(entity) == 0 && !EntityIsOutsideScreen(entity, 4))
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

/* Same as CheckUnk_53C_6 in code/code_080211F0.c. */
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
    struct EwramData_EntityData *other = &gEwramData->entityData[IMP_LINK_INDEX(entity)];

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
            IMP_GRAB_TIMER(entity) = 0;
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
                && (u8)IMP_GRAB_TIMER(entity)++ > 19)
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
    struct EwramData_EntityData *other = &gEwramData->entityData[IMP_LINK_INDEX(entity)];

    switch (entity->subState)
    {
        case 0:
            PlaySong(0x72);
            Enemy_DropRewards(entity);
            entity->palette = entity->unk_510.part8.basePalette;
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
    contactDamage.unk_4 = decoy;
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
    struct EwramData_EntityData *other = &gEwramData->entityData[IMP_LINK_INDEX(entity)];
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
