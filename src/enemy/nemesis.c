/*
 * Nemesis: body, partner, afterimages and attacks.
 */
#include "core.h"
#include "graphics.h"
#include "gameplay.h"
#include "structs/ewram.h"
#include "sound.h"
#include "bg_and_collision.h"
#include "entity_util.h"
#include "player_core.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "nemesis.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "damage_numbers.h"
#include "effects.h"
#include "entity_motion.h"
#include "constants/songs.h"

/* Nemesis_UpdatePositioning: twin of Alastor_UpdateApproach (approach a point near the player and rock);
 * deeper vertical target (+48/16), palette call in state 0; matched. */
#define DX(e) (Entity_GetTargetDistanceX(e))
#define DY(e) (Entity_GetTargetDistanceY(e))
#define ABS(x) ((x)<0?-(x):(x))
#define ANGLE(e) (e->ext.nemesis.angle)
#define PHASE(e) (e->ext.nemesis.phase)
#define FLAGS(e) (e->ext.nemesis.flags)

extern void *memcpy(void *, const void *, unsigned long);

void Nemesis_DeleteAfterimages(struct EwramData_EntityData *);
void NemesisPartner_Update(struct EwramData_EntityData *);

/**
 * @brief Turn and approach a point near the target, rock the affine angle and enter attack selection when both axis tolerances are reached.
 */
void Nemesis_UpdatePositioning(struct EwramData_EntityData *entity)
{
    s8 reachedX;
    s32 reachedY;
    reachedX=0;
    reachedY=0;
    if (DX(entity)>0)
        FLAGS(entity)|=2;
    else
        FLAGS(entity)&=~2;
    switch(entity->subState)
    {
        case 0:
            entity->objMode=0;
            HitBox_SetAttackRaw(entity,0x1010F8F8);
            entity->velX.whole=0x8000;
            entity->velY.whole=0x8000;
            if (DX(entity)>0)
                entity->velX.whole=-entity->velX.whole;
            if (DY(entity)>0)
                entity->velY.whole=-entity->velY.whole;
            PHASE(entity)=0;
            entity->ext.nemesis.turn=-100;
            entity->subState=2;
            break;
        case 1:
        case 2:
            if (DX(entity)>0)
            {
                if (Angle_TurnTowards(&ANGLE(entity),0x180,0x1000))
                    entity->subState++;
            }
            else
            {
                if (Angle_TurnTowards(&ANGLE(entity),0x180,0xF000))
                    entity->subState++;
            }
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,0,0,entity->affineIndex);
            break;
        case 3:
            entity->velY.whole=0x8000;
            entity->velX.whole=0x8000;
            if (DX(entity)>0)
                entity->velX.whole=-ABS(entity->velX.whole);
            if (DY(entity)+32>0)
                entity->velY.whole=-ABS(entity->velY.whole);
            if (ABS(DX(entity))>72)
                entity->posX.whole+=entity->velX.whole;
            else
                reachedX=1;
            if (ABS(DY(entity)+48)>16)
                entity->posY.whole+=entity->velY.whole;
            else
                reachedY=1;
            {
                s32 both;
                do { both=reachedY&reachedX; } while (0); /* FAKEMATCH: do/while(0) only raises the register-allocation weight */
            if (both==1 && entity->state!=2)
            {
                entity->state=2;
                entity->stage=0;
                entity->subState=0;
            }
            }
            if ((DX(entity)>=0 && entity->velX.whole>0) || (DX(entity)<=0 && entity->velX.whole<0))
            {
                entity->subState=0;
                break;
            }
            ANGLE(entity)=Sin((u16)PHASE(entity))/32;
            if (DX(entity)>0)
                ANGLE(entity)+=0x1000;
            else
                ANGLE(entity)+=-0x1000;
            {
                u32 phase=(u16)PHASE(entity);
                PHASE(entity)=phase+0x200;
            }
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,0,0,entity->affineIndex);
            if ((s16)entity->ext.nemesis.waitTimer!=0)
                entity->ext.nemesis.waitTimer--;
            else
                entity->ext.nemesis.waitTimer=RandomNumberGenerator()%180+10;
            break;
    }
}

/* Attack step of a part-driven enemy (Nemesis_UpdateAttackSelection, cf. Alastor Alastor_UpdateAttackSelection):
 * rotate the part through three target angles, then pick the next attack. */

#define SET(e) (e->ext.nemesis.attackSet)
#define INDEX(e) (e->ext.nemesis.attackIndex)
#define ATTACK(e) (sUnk_08528A34[SET(e)][INDEX(e)])
#define STEP(e) (e->ext.nemesis.turn)
#undef ABS
#define ABS(x) ((x) < 0 ? -(x) : (x))

static inline void SetState(struct EwramData_EntityData *entity, u8 state)
{
    if (entity->state != state)
    {
        entity->state = state;
        entity->stage = 0;
        entity->subState = 0;
    }
}

/**
 * @brief Read the randomized attack sequence, coordinate partner state 2 with angular transitions and dispatch the selected attack state.
 */
void Nemesis_UpdateAttackSelection(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[entity->ext.nemesis.partnerIndex];
    s32 speed = 0x20000;
    s32 target = 0xD800;

    if (entity->subState == 0)
    {
        if (INDEX(entity) == 9)
        {
            SET(entity) = RandomNumberGenerator() % 4;
            INDEX(entity) = 0;
        }
        if (ATTACK(entity) == 2)
        {
            SetState(entity, 5);
            INDEX(entity)++;
        }
    }
    switch (entity->subState)
    {
        case 0:
            if (!(entity->ext.nemesis.flags & 2))
            {
                speed = -speed;
                target = 0x10000 - target;
            }
            entity->posX.whole += speed;
            if (Angle_TurnTowards(&ANGLE(entity), (u16)ABS(STEP(entity) << 3), (u16)target))
            {
                SetState(part, 2);
                entity->subState++;
            }
            ObjAffine_Set(ANGLE(entity), 0x10000, 0x10000, 0, 0, entity->affineIndex);
            break;
        case 1:
            if (part->state == 0)
                entity->subState++;
            break;
        case 2:
            target = 0x9800;
            if (!(entity->ext.nemesis.flags & 2))
                target = 0x6800;
            if (Angle_TurnTowards(&ANGLE(entity), (u16)ABS(STEP(entity) << 4), (u16)target))
                entity->subState++;
            ObjAffine_Set(ANGLE(entity), 0x10000, 0x10000, 0, 0, entity->affineIndex);
            break;
        case 3:
            target = 0x2800;
            if (!(entity->ext.nemesis.flags & 2))
                target = 0xD800;
            if (Angle_TurnTowards(&ANGLE(entity), (u16)ABS(STEP(entity) << 4), (u16)target))
                entity->subState++;
            ObjAffine_Set(ANGLE(entity), 0x10000, 0x10000, 0, 0, entity->affineIndex);
            break;
        case 4:
        {
            s32 state = part->state;
            if (state != 0)
                break;
            if (INDEX(entity) == 9)
            {
                SET(entity) = RandomNumberGenerator() % 4;
                INDEX(entity) = state;
            }
            switch (ATTACK(entity))
            {
                case 0: SetState(entity, 3); break;
                case 1: SetState(entity, 4); break;
                case 2: SetState(entity, 5); break;
            }
            INDEX(entity)++;
            break;
        }
    }
}

/**
 * @brief Derive signed X and byte Y box offsets from the current angle and install the same packed attack and body box.
 */
void Nemesis_UpdateRotatedBoxes(struct EwramData_EntityData *entity)
{
    s8 x = FixedMultiply(Sin(entity->ext.nemesis.angle + 0x8000), 0x100000) >> 16;
    u8 y = FixedMultiply(Sin(entity->ext.nemesis.angle + 0x4000), 0x80000) >> 16;
    u32 packed;
    if (x > 0) x -= 0x10;
    packed = ((u8)x | (y << 8)) + 0x08100000;
    HitBox_SetAttackRaw(entity, packed);
    HitBox_SetBodyRaw(entity, packed);
}

/* Turning attack: three passes, then return to the ordinary state. */

#define LEFT(e) ((e)->ext.nemesis.attackStep)

static inline s16 GetTurn(struct EwramData_EntityData *e)
{ return e->ext.nemesis.turn; }
/**
 * @brief Run three table-driven turning passes with translation and affine rotation, then pause and return to positioning state 1.
 */
void Nemesis_UpdateTurningAttack(struct EwramData_EntityData *entity)
{
    s32 frames = 14;
    u16 turn[3];

    memcpy(turn, sNemesisTurn, sizeof(turn));
    Nemesis_UpdateRotatedBoxes(entity);
    switch (entity->subState)
    {
    case 0:
        LEFT(entity) = 3;
        entity->subState++;
    case 1:
        entity->ext.nemesis.turn = turn[3 - LEFT(entity)];
        if (!(FLAGS(entity) & 2))
            entity->ext.nemesis.turn = -GetTurn(entity);
        EntitySetVelocityFromAngle(entity, entity->ext.nemesis.angle + 0x4000, 0x10000);
        entity->timer = 0;
        entity->subState++;
    case 2:
        if (entity->timer++ > frames)
        {
            entity->timer = 0;
            entity->subState++;
            PlaySong(SE_SWING);
            break;
        }
        entity->posX.whole += entity->velX.whole * 3;
        entity->posY.whole += entity->velY.whole * 3;
        break;
    case 3:
        if (LEFT(entity) == 1)
        {
            entity->subState = 4;
            entity->timer = 0;
            break;
        }
        if (entity->timer++ > frames / 2)
        {
            entity->timer = 0;
            /* Re-read the remaining-pass byte after resetting the timer,
             * as the original does; do not forward the earlier comparison. */
            LEFT(entity) = *(volatile u8 *)&LEFT(entity) - 1; /* FAKEMATCH: volatile read forces the reload of the original */
            if (LEFT(entity) == 0)
                entity->subState = 4;
            else
                entity->subState = 1;
            break;
        }
        entity->ext.nemesis.angle += entity->ext.nemesis.turn;
        ObjAffine_Set(entity->ext.nemesis.angle, 0x10000, 0x10000, 0, 0, entity->affineIndex);
        EntitySetVelocityFromAngle(entity, entity->ext.nemesis.angle + 0x4000, 0x10000);
        entity->posX.whole -= entity->velX.whole * 6;
        entity->posY.whole -= entity->velY.whole * 6;
        break;
    case 4:
        if (entity->timer++ > 0x28)
        {
            entity->timer = 0;
            if (entity->state != 1)
            {
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
            LEFT(entity) = 0;
        }
        break;
    }
}

/* Scripted movement along a step table. */

#undef STEP
#define STEP(e) ((e)->ext.nemesis.attackStep)

/**
 * @brief Distribute each movement-table step over its frame count, update rotation and boxes, and return to positioning after the terminator and pause.
 */
void Nemesis_UpdateScriptedAttack(struct EwramData_EntityData *entity)
{
    struct Unk_C70F0_Step *table = sNemesisScriptedAttackSteps;
    struct Unk_C70F0_Step *step = &table[STEP(entity)];
    u8 frames;
    s32 mirrored;
    s32 dx;

    Nemesis_UpdateRotatedBoxes(entity);
    switch (entity->subState)
    {
    case 0:
        STEP(entity) = 0;
        step = table;
        PlaySong(SE_SWING);
        entity->subState++;
    case 1:
        frames = entity->timer = step->frames;
        dx = step->dx;
        mirrored = FLAGS(entity) & 2;
        if (mirrored)
            dx = -dx;
        dx <<= 16;
        entity->velX.whole = dx / frames;
        entity->velY.whole = (step->dy << 16) / frames;
        entity->ext.nemesis.turn = step->rotation / frames;
        if (!mirrored)
            entity->ext.nemesis.turn = -entity->ext.nemesis.turn;
        entity->subState++;
    case 2:
        entity->posX.whole += entity->velX.whole;
        entity->posY.whole += entity->velY.whole;
        entity->ext.nemesis.angle += entity->ext.nemesis.turn;
        ObjAffine_Set(entity->ext.nemesis.angle, 0x10000, 0x10000, 0, 0, entity->affineIndex);
        if (--entity->timer == 0)
        {
            STEP(entity)++;
            if (table[STEP(entity)].dx == 0x7FFF)
                entity->subState = 5;
            else
                entity->subState = 1;
            entity->timer = 0;
            PlaySong(SE_SWING);
        }
        break;
    case 5:
        if (entity->timer++ > 0x28)
        {
            entity->timer = 0;
            if (entity->state != 1)
            {
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
            STEP(entity) = 0;
        }
        break;
    }
}

/* Lunge towards the target. */

/**
 * @brief Spin in place, aim toward the target, lunge for the computed timer and return to positioning; update rotation and boxes throughout.
 */
void Nemesis_UpdateSpinLunge(struct EwramData_EntityData *entity)
{
    struct BgLayerState *bg = &gEwramData->bgInfo[1];
    s32 targetX, targetY, dist;

    targetX = Target_GetX(entity) + bg->xPos.whole;
    targetY = Target_GetY(entity) + bg->yPos.whole - 0x200000;
    /* computed but unused in the original */
    dist = ABS((s32)(targetX - GetEntityRoomXPositionWhole(entity)));
    dist = ABS((s32)(targetY - GetEntityRoomYPositionWhole(entity)));
    Nemesis_UpdateRotatedBoxes(entity);
    switch (entity->subState)
    {
    case 0:
        entity->accelY.whole = 0;
        entity->accelX.whole = 0;
        entity->velY.whole = 0;
        entity->velX.whole = 0;
        entity->ext.nemesis.turn = 0x1000;
        entity->timer = 0;
        entity->subState++;
    case 1:
        if (!(entity->timer % 16))
            PlaySong(SE_SWING);
        if (entity->timer++ > 0x40)
        {
            entity->ext.nemesis.turn = 0;
            entity->ext.nemesis.angle = (u32)Entity_GetTargetAngleAbove16(entity) + 0xC000;
            entity->velX.whole = FixedMultiply(Sin(entity->ext.nemesis.angle + 0x8000), 0x20000);
            entity->velY.whole = FixedMultiply(Sin(entity->ext.nemesis.angle + 0x4000), 0x20000);
            entity->timer = ABS((Target_GetX(entity) - entity->posX.whole) / entity->velX.whole);
            entity->subState++;
            PlaySong(SE_SWING);
        }
        break;
    case 2:
        if (--entity->timer == 0)
        {
            entity->subState = 3;
            entity->timer = 0;
        }
        break;
    case 3:
    case 4:
        if (entity->state != 1)
        {
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
    entity->ext.nemesis.angle += entity->ext.nemesis.turn;
    ObjAffine_Set(entity->ext.nemesis.angle, 0x10000, 0x10000, 0, 0, entity->affineIndex);
}

/* Arc flight while the partner attacks. */
/* bit 3: target to the left, bit 5: dead */
#define TIMER(e) (e->ext.nemesis.phase)

/* 8.8 x 8.8 products; the two operand orders give the original registers */
static inline s32 FixMul(s32 a, s32 b)
{
    return (a >> 8) * (b >> 8);
}

static inline s32 FixMulR(s32 a, s32 b)
{
    s32 x = a >> 8;
    return (b >> 8) * x;
}

static inline s32 Mul(s32 a, s32 b)
{
    return a * b;
}

/**
 * @brief Move along an arc while the partner reacts, then await partner state 0 and return to positioning or death according to health.
 */
void Nemesis_UpdatePartnerFlight(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *partner = &gEwramData->entityData[(u8)entity->ext.nemesis.partnerIndex];
    s32 angle, speedX, speedY;
    s16 t;

    /* set before the switch: the original rematerialises it later */
    speedY = 0x18000;

    switch (entity->subState)
    {
    case 0:
        entity->timer = 0;
        TIMER(entity) = 0;
        if (gEwramData->playerState.player->posX.whole < entity->posX.whole)
            FLAGS(entity) |= 8;
        else
            FLAGS(entity) &= ~8;
        entity->subState++;
    case 1:
        angle = 0xD000;
        speedX = 0x20000;
        t = TIMER(entity);
        if (!(FLAGS(entity) & 8))
            speedX = -speedX;
        entity->posX.whole += FixMul(speedX, Sin(angle + 0x4000));
        entity->posY.whole += FixMulR(speedY, Sin(angle)) + Mul(t, 0xF00);
        TIMER(entity)++;
        if (partner->state == 4)
            entity->subState++;
        break;
    case 2:
        if (partner->state == 0)
        {
            if ((s16)entity->hp <= 0)
            {
                if (entity->state != 0xFF)
                {
                    entity->state = 0xFF;
                    entity->stage = 0;
                    entity->subState = 0;
                }
            }
            else if (entity->state != 1)
            {
                entity->state = 1;
                entity->stage = 0;
                entity->subState = 0;
            }
        }
        break;
    }
    if ((s16)entity->hp <= 0)
        FLAGS(entity) |= 0x20;
}

/* Falling-away death of a linked part. */

/**
 * @brief Delete afterimages, fall and emit effects; free the affine slot and request deletion of self and partner at the Y or timer limit.
 */
void Nemesis_UpdateDeath(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
    case 0:
        PlaySong(0x72);
        entity->palette = entity->basePalette;
        entity->velY.whole = 0x8000;
        entity->accelY.whole = 0x800;
        entity->timer = 0;
        entity->subState++;
        Nemesis_DeleteAfterimages(entity);
    case 1:
        entity->posY.whole += entity->velY.whole;
        entity->velY.whole += entity->accelY.whole;
        if ((s16)entity->posY.part16.integer > 0xAA || entity->timer++ > 0x40)
        {
            ObjAffine_Free(entity->affineIndex);
            entity->deleteRequested = 1;
            (gEwramData->entityData + entity->ext.nemesis.partnerIndex)->deleteRequested = 1;
        }
        break;
    }
    if ((u8)((u32)entity->timer % 7) == 0)
        Effect_SpawnRandom(entity->posX.whole, entity->posY.whole, 0x100000, 0x100000, entity->spriteLayer - 1);
    entity->hidden = entity->timer % 2;
}

/* Hit callback of a two-part enemy. */

/* the partner entity, slot unk_4F6 (see Nemesis_CreatePartner) */
#define LINKED(e) (*(gEwramData->entityData + e->ext.nemesis.partnerIndex))

/**
 * @brief Apply damage and flash; when the hit counter or health condition requires it, start partner reaction and state 6, then reveal self and afterimages.
 */
void Nemesis_OnHit(struct EwramData_EntityData *entity, s32 hit, u16 *data)
{
    s32 damage = Hit_ResolveDamage(entity, (struct EwramData_EntityData *)hit, data, Player_CalculateAttackPower(data[2]));

    /* hp read before the sign extension, as in Werewolf_OnHit */
    {
        u16 hp = entity->hp;
        damage <<= 16;
        damage >>= 16;
        entity->hp = hp - damage;
    }
    Entity_ShowDamage(entity, (s16)damage, 0);
    entity->hitFlashTimer = 8;
    if (entity->ext.nemesis.hitsBeforeReact != 0 && (s16)entity->hp > 0)
        entity->ext.nemesis.hitsBeforeReact--;
    else
    {
        if ((s16)entity->hp <= 0)
        {
            Enemy_DropRewards(entity);
            PlaySong(0x1C3);
        }
        if (LINKED(entity).state != 3)
        {
            LINKED(entity).state = 3;
            LINKED(entity).subState = LINKED(entity).stage = 0;
        }
        entity->state = 6;
        entity->subState = 0;
        entity->ext.nemesis.hitsBeforeReact = 3;
    }
    entity->hidden = 0;
    Nemesis_SetAfterimagesHidden(entity, 0);
}

/* Linked child creation. */

/**
 * @brief Allocate and attach the linked partner, store mutual slot references and run its first update; return 0 if allocation or sprite attachment fails.
 */
s32 Nemesis_CreatePartner(struct EwramData_EntityData *parent)
{
    EntityFunc update = NemesisPartner_Update;
    struct EwramData_EntityData *e = EntityCreateInRange(ENTITY_SLOT_EFFECT_FIRST, ENTITY_SLOT_EFFECT_LAST, update);

    if (e == NULL)
        return 0;
    if (!Sprite_Attach(e, sNemesisSpriteSheet, ObjGfx_Find(sNemesisGfx), ObjPalette_FindOrLoad(sNemesisPalette)))
        return 0;
    ENTITY_PARENT_INDEX(e) = ENTITY_INDEX(parent);
    parent->ext.nemesis.partnerIndex = ENTITY_INDEX(e);
    e->objMode = 1;
    e->drawFunc = Sprite_DrawEntity;
    e->scrollWithBg = 1;
    e->spriteLayer = parent->spriteLayer + 2;
    if (e->state != 1)
    {
        e->state = 1;
        e->stage = 0;
        e->subState = 0;
    }
    update(e);
    return 1;
}

/* Linked part (NemesisPartner_Update, cf. Alastor part AlastorPart_Update): follows its
 * owner, two attack poses with timed return to the blinking rest state. */

/**
 * @brief Follow the owner in the initial pose or execute owner-triggered attack/reaction poses and a timed blinking return to hidden state 0.
 */
void NemesisPartner_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    if (Enemy_IsStunned(entity))
        return;
    switch (entity->state)
    {
        case 0:
            entity->hidden=1;
            break;
        case 1:
            entity->hidden=0;
            entity->animFrame=1;
            entity->posX.whole=owner->posX.whole;
            entity->posY.whole=owner->posY.whole;
            Entity_FaceTarget(entity);
            break;
        case 2:
            switch(entity->subState)
            {
                case 0:
                    entity->posX.whole=owner->posX.whole;
                    Entity_FaceTarget(entity);
                    entity->flipX=1-entity->flipX;
                    entity->hidden=0;
                    entity->animFrame=2;
                    entity->timer=0;
                    entity->subState++;
                case 1:
                    if (entity->flipX)
                        entity->posX.whole=owner->posX.whole-0x90000;
                    else
                        entity->posX.whole=owner->posX.whole+0x90000;
                    entity->posY.whole=owner->posY.whole+0x250000;
                    if (entity->timer++>9)
                        SetState(entity, 4);
                    break;
            }
            break;
        case 3:
            switch(entity->subState)
            {
                case 0:
                    entity->hidden=0;
                    entity->animFrame=3;
                    entity->timer=0;
                    entity->subState++;
                    if (gEwramData->playerState.player->posX.whole<owner->posX.whole)
                        entity->flipX=0;
                    else
                        entity->flipX=1;
                case 1:
                    if (!entity->flipX)
                        entity->posX.whole=owner->posX.whole-0xA0000;
                    else
                        entity->posX.whole=owner->posX.whole+0xA0000;
                    entity->posY.whole=owner->posY.whole-0x40000;
                    if (entity->timer++>39)
                        SetState(entity, 4);
                    break;
            }
            break;
        case 4:
            switch(entity->subState)
            {
                case 0:
                    entity->timer=0;
                    entity->subState++;
                case 1:
                    entity->hidden=((u32)-(u8)((u32)entity->timer%3))>>31;
                    if (entity->timer++>30)
                    {
                        SetState(entity, 0);
                        entity->timer=0;
                    }
                    break;
            }
            break;
        case 5:
            break;
    }
}

/* Effect_FollowParentHitBox. */

/**
 * @brief C799C | Follow the parent (entity unk_4F5) at the offset returned
 * by HitBox_GetBoxes; state 0 first takes the loaded graphics sCommonObjectGfx
 * (palette 7, animation 0).
 */
void Effect_FollowParentHitBox(struct EwramData_EntityData *entity)
{
    struct SpriteBox attackBox;
    struct SpriteBox bodyBox;
    struct EwramData_EntityData *parent = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];

    HitBox_GetBoxes(&attackBox, &bodyBox, parent);
    switch (entity->state)
    {
        case 0:
        {
            s32 gfx = ObjGfx_Find(sCommonObjectGfx);

            Sprite_Attach(entity, sCommonObjectSpriteSheet, gfx, 7);
            Anim_StartIndexed(entity, 0, 0, 0);
            entity->drawFunc = Sprite_DrawEntity;
            entity->state++;
        }
        case 1:
            entity->posX.part16.integer = parent->posX.part16.integer + attackBox.x;
            entity->posY.part16.integer = parent->posY.part16.integer + attackBox.y;
            break;
    }
}

/* Killer Doll child visibility/deletion helpers, reconstructed from the
 * original assembly. */

/**
 * @brief Set hidden on each nonzero slot in the three stored afterimage slots.
 */
void Nemesis_SetAfterimagesHidden(struct EwramData_EntityData *entity, s32 hidden)
{
    u8 children[3] = {
        entity->ext.nemesis.afterimage0,
        entity->ext.nemesis.afterimage1,
        entity->ext.nemesis.afterimage2
    };
    s32 i;

    for (i = 0; i < 3; i++)
        if (children[i] != 0)
        {
            struct EwramData_EntityData *child = &gEwramData->entityData[children[i]];
            child->hidden = hidden;
        }
}

/**
 * @brief Request deletion of each nonzero slot in the three stored afterimage slots.
 */
void Nemesis_DeleteAfterimages(struct EwramData_EntityData *entity)
{
    u8 children[3] = {
        entity->ext.nemesis.afterimage0,
        entity->ext.nemesis.afterimage1,
        entity->ext.nemesis.afterimage2
    };
    s32 i;

    for (i = 0; i < 3; i++)
        if (children[i] != 0)
        {
            struct EwramData_EntityData *child = &gEwramData->entityData[children[i]];
            child->deleteRequested = 1;
        }
}

/**
 * @brief Empty state-7 handler. Its gameplay purpose remains unresolved.
 */
void Nemesis_UpdateState7(struct EwramData_EntityData *entity)
{
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Nemesis_OnContact = Enemy_DamagePlayerOnContact */

/**
 * @brief Submit the entity attack contact record with mode fields 0, 0, 0.
 */
void Nemesis_OnContact(struct EwramData_EntityData *entity)
{
    Entity_DamagePlayerOnContact(entity, 0, 0, 0);
}

/* Nemesis_CreateAfterimages. */

/**
 * @brief Create three afterimages with zero countdown, history indices 9, 12 and 6, and flag 2; store their slots.
 */
void Nemesis_CreateAfterimages(struct EwramData_EntityData *entity)
{
    entity->ext.nemesis.afterimage0 = EntityGetSlot(Afterimage_Create(entity, 0, 9, 2));
    entity->ext.nemesis.afterimage1 = EntityGetSlot(Afterimage_Create(entity, 0, 0xC, 2));
    entity->ext.nemesis.afterimage2 = EntityGetSlot(Afterimage_Create(entity, 0, 6, 2));
}
