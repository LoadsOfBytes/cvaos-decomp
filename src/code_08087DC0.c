/* Peeping Eye: main update, creation, movement and animation, the hit
 * reaction and its parts (eyeball, pupil and the chain of six links).
 * The whole range 0x08087DC0..0x08088D27 is now C. */
#include "code_08009A0.h"
#include "code_080D73B8.h"
#include "syscalls.h"
#include "code/code_080211F0.h"
#include "code/code_08039340.h"
#include "code/code_08040A38.h"
#include "code/code_08060B98.h"
#include "structs/ewram.h"

extern void Hit_CheckWithPlayer(void *, struct EwramData_EntityData *);
extern void Entity_ShowDamage(struct EwramData_EntityData *, s32, s32);
extern void Hit_EnableContact(struct EwramData_EntityData *, s32, void *, void *);
extern void Enemy_DropRewards(struct EwramData_EntityData *);
extern s32 FixedMultiply(s32, s32);
extern s32 Enemy_IsStunnedNoFlash(struct EwramData_EntityData *);
extern void Enemy_UpdateHitFlash(struct EwramData_EntityData *);
extern void Entity_SnapToGround(struct EwramData_EntityData *);
extern void Enemy_ResetFromTemplate(struct EwramData_EntityData *);
extern s32 Hit_ResolveDamage(struct EwramData_EntityData *, struct EwramData_EntityData *, u16 *, s16);
extern s32 Target_GetX(struct EwramData_EntityData *);
extern s32 Target_GetY(struct EwramData_EntityData *);
extern s32 Entity_IsTargetRight(struct EwramData_EntityData *);
extern s32 Target_IsInRect(s16, s16, s16, s16);
extern s32 Anim_GetMaxBoxCount(struct EwramData_EntityData *);
extern s32 Entity_IsTimeStopped(struct EwramData_EntityData *);
extern s32 Enemy_IsDamaged(struct EwramData_EntityData *);
extern void Effect_SpawnAt(s32, s32, s32);
extern struct EwramData_EntityData *Entity_CreateObject(u32 *updateFunc);
extern void EntityDampVelocityX(struct EwramData_EntityData *, s32);
extern void EntityDampVelocityY(struct EwramData_EntityData *, s32);
extern void EntityAccelerateAndMove(struct EwramData_EntityData *, s32);
extern struct EwramData_EntityData *EnemyPeepingEyeCreateEyeball(struct EwramData_EntityData *);
extern struct EwramData_EntityData *PeepingEye_CreatePupil(struct EwramData_EntityData *);
extern u8 PeepingEye_LoadGraphics(struct EwramData_EntityData *);
extern void PeepingEye_UpdateIdle(struct EwramData_EntityData *);
extern void PeepingEye_DamagePlayerOnContact(struct EwramData_EntityData *);
void PeepingEye_UpdateMove(struct EwramData_EntityData *entity);
void PeepingEye_UpdatePreview(struct EwramData_EntityData *entity);
void PeepingEye_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit);
struct EwramData_EntityData *PeepingEye_CreateChainLink(struct EwramData_EntityData *owner, struct EwramData_EntityData *previous, s32 index);
void PeepingEyeChainLink_Update(struct EwramData_EntityData *entity);

struct SingleFrameAnimation {
    u16 count;
    u16 format;
    u8 frame;
    u8 duration;
    u16 unk_6;
};
extern struct SingleFrameAnimation sPeepingEyeFrameAnim;
extern u8 sPeepingEyeGfx[];
extern u8 sPeepingEyeSpriteSheet[];
/* Animations are (frame, duration) byte pairs ending with frame 0xFF:
 * the opening animation and the idle/look-up/look-down/hit set. */
extern u8 sPeepingEyeMoveSteps[];
extern u8 *sPeepingEyeAnimSets[];
#define STEP_FRAME(a, i) ((a)[(i) * 2])
#define STEP_DURATION(a, i) ((a)[(i) * 2 + 1])

/* Peeping Eye body */
#define ANIM_FLAGS(e) (e->ext.peepingEyeBody.animFlags) /* bit 0: frame override, bit 1: eye open */
#define ANIM_FRAME(e) (e->ext.peepingEyeBody.animFrame)
#define ANIM_INDEX(e) (*(s8 *)&e->ext.peepingEyeBody.animIndex)
#define ANIM_CLOCK(e) (e->ext.peepingEyeBody.animClock)
#define ANIM_SET(e) (*(u8 *)&e->ext.peepingEyeBody.animSet)
#define HIT_FLAG(e) (e->ext.peepingEyeBody.hitFlag) /* set by PeepingEye_OnHit */
#define TARGET_X(e) (e->ext.peepingEyeBody.targetX) /* room coordinates */
#define TARGET_Y(e) (e->ext.peepingEyeBody.targetY)
#define ACCEL_X(e) ((e)->accelX.whole)
#define ACCEL_Y(e) ((e)->accelY.whole)
/* Spawn parameter (EnemyPeepingEyeCreate also picks layer 0x1D when it is
 * non-zero). Off-screen despawning and the death collision only apply when
 * it is zero. TODO: meaning. */
#define SPAWN_PARAM(e) ((e)->unk_514.part16.spawnParam0)
/* States 2 and 3 are the death sequence. */
#define NOT_DYING(e) ((u8)((e)->state - 2) > 1)

/* Chain links */
#define LINK_INDEX(e) ((s8)e->ext.peepingEyeLink.index)
#define LINK_TIMER(e) (e->ext.peepingEyeLink.timer)
#define LINK_ANGLE(e) (e->ext.peepingEyeLink.angle)
#define LINK_LENGTH(e) (e->ext.peepingEyeLink.length)
#define LINK_MAX_LENGTH(e) (*(s32 *)&e->ext.peepingEyeLink.maxLength)

/* Peeping Eye: dispatch the states, draw the frame chosen by the movement
 * code and fall apart when killed. */
void EnemyPeepingEyeUpdate(struct EwramData_EntityData *entity)
{
    u8 collision[48];
    struct SingleFrameAnimation animation = sPeepingEyeFrameAnim;

    ANIM_FLAGS(entity) = (u8)ANIM_FLAGS(entity) & 0xFE;
    if (entity->state != 2)
    {
        if (SPAWN_PARAM(entity) == 0 && EntityIsOutsideScreen(entity, 4)) goto finished;
        if (Entity_IsInRect(entity, -16, 0, 272, 176)) Player_TrackEntity(entity);
    }
    if (EntityIsOutsideScreen(entity, 4)) entity->hidden = 1;
    else entity->hidden = 0;
    if (NOT_DYING(entity)) Enemy_UpdateHitFlash(entity);
    if (Entity_IsTimeStopped(entity) && NOT_DYING(entity))
    {
        if (SPAWN_PARAM(entity) == 0) Hit_CheckWithPlayer(collision, entity);
        goto finished;
    }
    switch (entity->state)
    {
    case 0:
        PeepingEye_UpdateMove(entity);
        break;
    case 1:
        PeepingEye_UpdateIdle(entity);
        break;
    case 2:
        ANIM_FLAGS(entity) |= 1;
        ANIM_FRAME(entity) = 0;
        entity->hidden = 1;
        if (entity->stage == 0)
        {
            entity->stage++;
            Effect_SpawnAt(entity->posX.whole, entity->posY.whole, entity->spriteLayer - 1);
            entity->hidden = 1;
        }
        if ((s8)--entity->timer <= 0)
        {
            entity->state = 3;
            entity->timer = 0;
        }
        break;
    case 3:
        entity->hidden = 1;
        if (entity->timer++ > 2) entity->deleteRequested = 1;
        break;
    case 4:
        PeepingEye_UpdatePreview(entity);
        if (ANIM_FLAGS(entity) & 1)
        {
            Anim_Start(entity, (struct AnimScript *)&animation, 0, 1);
            animation.frame = ANIM_FRAME(entity);
        }
        Anim_Update(entity);
        goto finished;
    }
    if (ANIM_FLAGS(entity) & 1)
    {
        Anim_Start(entity, (struct AnimScript *)&animation, 0, 1);
        animation.frame = ANIM_FRAME(entity);
        Anim_Update(entity);
    }
    else
        Anim_Update(entity);
    if (NOT_DYING(entity) && SPAWN_PARAM(entity) == 0)
        Hit_CheckWithPlayer(collision, entity);
finished:
    return;
}

/* Create the Peeping Eye with its eyeball, pupil and a chain of six links
 * that swing in one of two random directions. */
void EnemyPeepingEyeCreate(struct EwramData_EntityData *entity)
{
    u32 box = 0x0E0EF9F9; /* bytes x -7, y -7, w 14, h 14; not an address */
    struct EwramData_EntityData *link;
    s32 reverse;
    s32 i;

    if (!PeepingEye_LoadGraphics(entity)) goto fail;
    Anim_StartIndexed(entity, 0, 0, 1);
    Anim_Update(entity);
    Enemy_ResetFromTemplate(entity);
    entity->spriteLayer = 3;
    if (SPAWN_PARAM(entity)) entity->spriteLayer = 0x1D;
    if (!EnemyPeepingEyeCreateEyeball(entity)) goto fail;
    if (!PeepingEye_CreatePupil(entity)) goto fail;
    link = entity;
    reverse = RandomNumberGenerator() % 2;
    for (i = 0; i < 6; i++)
    {
        link = PeepingEye_CreateChainLink(entity, link, i);
        if (link != NULL)
        {
            LINK_LENGTH(link) = 0;
            if (!reverse) LINK_ANGLE(link) = (i + 1) * 5000;
            else LINK_ANGLE(link) = (6 - i) * 5000;
            LINK_TIMER(link) = (i + 1) * 6;
            switch (i)
            {
            case 0:
                link->animFrame = 0xB;
                LINK_MAX_LENGTH(link) = 0xC0000;
                break;
            case 1:
                link->animFrame = 0xB;
                LINK_MAX_LENGTH(link) = 0xA0000;
                break;
            case 2:
            case 3:
                link->animFrame = 0xC;
                LINK_MAX_LENGTH(link) = 0x80000;
                break;
            case 4:
                link->animFrame = 0xD;
                LINK_MAX_LENGTH(link) = 0x70000;
                break;
            case 5:
                link->animFrame = 0xE;
                LINK_MAX_LENGTH(link) = 0x30000;
                break;
            }
        }
        else
        {
        fail:
            EntityDelete(entity);
            return;
        }
    }
    if (gEwramData->gameState.bestiaryPreview)
    {
        entity->state = 4;
        entity->posY.whole -= 0x300000;
        return;
    }
    if (Anim_GetMaxBoxCount(entity) > 1)
        Hit_EnableContact(entity, 8, PeepingEye_DamagePlayerOnContact, PeepingEye_OnHit);
    else
        Hit_Enable(entity, 8, PeepingEye_DamagePlayerOnContact, (s32)PeepingEye_OnHit);
    HitBox_SetAttack(entity, &box);
    HitBox_SetBody(entity, &box);
    entity->hit.flags &= ~4;
    entity->hit.flags &= ~2;
    Entity_SnapToGround(entity);
    entity->posY.whole -= 0x80000;
    entity->state = 0;
}

/* Peeping Eye movement: wait for the player, open the eye, then keep
 * flying to random points around the player (room coordinates). */
void PeepingEye_UpdateMove(struct EwramData_EntityData *entity)
{
    struct BgLayerState *camera = &gEwramData->bgInfo[1];
    u8 index;
    u8 *anim;
    s32 x, y;

    switch (entity->subState)
    {
    case 0:
        if (Target_IsInRect(entity->posX.part16.integer - 90, entity->posY.part16.integer - 100, 180, 200)
            || Enemy_IsDamaged(entity))
        {
            entity->subState++;
            entity->timer = 0;
            ANIM_CLOCK(entity) = 0;
            ANIM_INDEX(entity) = 0;
        }
        break;
    case 1:
        ANIM_FLAGS(entity) |= 1;
        ANIM_CLOCK(entity)++;
        if (ANIM_CLOCK(entity) >= STEP_DURATION(sPeepingEyeMoveSteps, ANIM_INDEX(entity)))
        {
            ANIM_CLOCK(entity) = 0;
            index = entity->ext.peepingEyeBody.animIndex;
            entity->ext.peepingEyeBody.animIndex = index + 1;
            if (STEP_FRAME(sPeepingEyeMoveSteps, ANIM_INDEX(entity)) == 0xFF)
            {
                entity->subState++;
                ANIM_FLAGS(entity) |= 2;
                ANIM_INDEX(entity) = index;
                ANIM_FRAME(entity) = STEP_FRAME(sPeepingEyeMoveSteps, ANIM_INDEX(entity));
                ANIM_INDEX(entity) = 0;
                ANIM_SET(entity) = 0;
                ANIM_CLOCK(entity) = RandomNumberGenerator() % 64;
                HIT_FLAG(entity) = 0;
                entity->flipX = Entity_IsTargetRight(entity);
                {
                    s32 target = Target_GetX(entity);
                    s32 offset = camera->xPos.whole + (entity->flipX ? -0x400000 : 0x400000);
                    TARGET_X(entity) = target + offset;
                }
                {
                    s32 target = Target_GetY(entity);
                    s32 offset = camera->yPos.whole - 0x400000;
                    TARGET_Y(entity) = target + offset;
                }
                entity->velX.whole = 0;
                entity->velY.whole = 0;
                entity->subState++;
                entity->timer = 0;
                break;
            }
        }
        ANIM_FRAME(entity) = STEP_FRAME(sPeepingEyeMoveSteps, ANIM_INDEX(entity));
        break;
    case 2:
    {
        s32 offset = RandomNumberGenerator() % 0x300000 + 0x100000;
        y = Target_GetX(entity);
        x = camera->xPos.whole;
        if (RandomNumberGenerator() % 2 == 0)
            TARGET_X(entity) = y - offset + x;
        else
            TARGET_X(entity) = y + offset + x;
        y = Target_GetY(entity);
        y -= RandomNumberGenerator() % 0x300000;
        {
            s32 base = camera->yPos.whole - 0x200000;
            y += base;
        }
        TARGET_Y(entity) = y;
        entity->subState++;
        entity->timer = 0;
    }
    case 3:
        ANIM_FLAGS(entity) |= 1;
        ANIM_CLOCK(entity)++;
        anim = sPeepingEyeAnimSets[ANIM_SET(entity)];
        if (ANIM_CLOCK(entity) >= STEP_DURATION(anim, ANIM_INDEX(entity)))
        {
            ANIM_CLOCK(entity) = 0;
            ANIM_INDEX(entity)++;
            if (STEP_FRAME(anim, ANIM_INDEX(entity)) == 0xFF)
            {
                ANIM_INDEX(entity) = 0;
                ANIM_CLOCK(entity) = RandomNumberGenerator() % 64;
                if (ANIM_SET(entity) == 0)
                {
                    /* Look up or down when the target is above or below. */
                    s32 pixelX, pixelY;
                    u16 direction;
                    x = entity->posX.whole + 0x10000;
                    y = entity->posY.whole;
                    x = Target_GetX(entity) - x;
                    {
                        s32 target = Target_GetY(entity);
                        y += 0x170000;
                        y = target - y;
                    }
                    if (x < 0) pixelX = (s16)(-((-x) >> 16));
                    else pixelX = x >> 16;
                    if (y < 0) pixelY = (s16)(-((-y) >> 16));
                    else pixelY = y >> 16;
                    direction = ArcTan2(pixelX, pixelY);
                    if ((u32)(direction - 0x2000) <= 0x4000)
                        ANIM_SET(entity) = 2;
                    else if ((u32)(direction - 0xA000) <= 0x4000)
                        ANIM_SET(entity) = 3;
                }
                else
                    ANIM_SET(entity) = 0;
            }
        }
        if (HIT_FLAG(entity))
        {
            HIT_FLAG(entity) = 0;
            if (ANIM_SET(entity) != 1)
            {
                entity->flipX = Entity_IsTargetRight(entity);
                /* stored through the signed member, unlike the other writes */
                entity->ext.peepingEyeBody.animSet = 1;
                ANIM_INDEX(entity) = 0;
                ANIM_CLOCK(entity) = 0;
            }
        }
        anim = sPeepingEyeAnimSets[ANIM_SET(entity)];
        ANIM_FRAME(entity) = STEP_FRAME(anim, ANIM_INDEX(entity));
        x = TARGET_X(entity) - (entity->posX.whole + camera->xPos.whole);
        y = TARGET_Y(entity) - (entity->posY.whole + camera->yPos.whole);
        if (x > 0x40000) ACCEL_X(entity) = 0x800;
        else if (x < -0x40000) ACCEL_X(entity) = -0x800;
        else ACCEL_X(entity) = 0;
        if (y > 0x40000) ACCEL_Y(entity) = 0x800;
        else if (y < -0x40000) ACCEL_Y(entity) = -0x800;
        else ACCEL_Y(entity) = 0;
        EntityDampVelocityX(entity, 0x20);
        EntityDampVelocityY(entity, 0x20);
        EntityAccelerateAndMove(entity, 0x20000);
        if ((x < 0 ? -x : x) <= 0x3FFFF && (y < 0 ? -y : y) <= 0x3FFFF)
            entity->subState = 2;
        if (entity->timer++ > 0x77)
            entity->subState = 2;
        break;
    case 4:
        break;
    }
}

/* Opening and idle/look animations without movement: state 4 of
 * EnemyPeepingEyeUpdate, chosen by EnemyPeepingEyeCreate when bestiaryPreview is set. */
void PeepingEye_UpdatePreview(struct EwramData_EntityData *entity)
{
    u8 *anim;
    u8 index;

    switch (entity->subState)
    {
    case 0:
        if (entity->timer++ > 29) entity->subState++;
        break;
    case 1:
        ANIM_FLAGS(entity) |= 1;
        ANIM_CLOCK(entity)++;
        if (ANIM_CLOCK(entity) >= STEP_DURATION(sPeepingEyeMoveSteps, ANIM_INDEX(entity)))
        {
            ANIM_CLOCK(entity) = 0;
            index = entity->ext.peepingEyeBody.animIndex;
            entity->ext.peepingEyeBody.animIndex = index + 1;
            if (STEP_FRAME(sPeepingEyeMoveSteps, ANIM_INDEX(entity)) == 0xFF)
            {
                entity->subState++;
                ANIM_FLAGS(entity) |= 2;
                ANIM_INDEX(entity) = index;
                ANIM_FRAME(entity) = STEP_FRAME(sPeepingEyeMoveSteps, ANIM_INDEX(entity));
                ANIM_INDEX(entity) = 0;
                ANIM_SET(entity) = 0;
                ANIM_CLOCK(entity) = RandomNumberGenerator() % 64;
                HIT_FLAG(entity) = 0;
                break;
            }
        }
        ANIM_FRAME(entity) = STEP_FRAME(sPeepingEyeMoveSteps, ANIM_INDEX(entity));
        break;
    case 2:
    case 3:
        ANIM_FLAGS(entity) |= 1;
        ANIM_CLOCK(entity)++;
        anim = sPeepingEyeAnimSets[ANIM_SET(entity)];
        if (ANIM_CLOCK(entity) >= STEP_DURATION(anim, ANIM_INDEX(entity)))
        {
            ANIM_CLOCK(entity) = 0;
            ANIM_INDEX(entity)++;
            if (STEP_FRAME(anim, ANIM_INDEX(entity)) == 0xFF)
            {
                ANIM_INDEX(entity) = 0;
                ANIM_CLOCK(entity) = RandomNumberGenerator() % 64;
                ANIM_SET(entity) = 0;
            }
        }
        anim = sPeepingEyeAnimSets[ANIM_SET(entity)];
        ANIM_FRAME(entity) = STEP_FRAME(anim, ANIM_INDEX(entity));
        break;
    case 4:
        break;
    }
}

/* Bounce away from a hit, using the direction between the entities. */
static inline void Bounce(struct EwramData_EntityData *entity, struct EwramData_EntityData *other)
{
    s32 x = entity->posX.whole - other->posX.whole;
    s32 y = entity->posY.whole - other->posY.whole;
    s32 pixelX, pixelY;
    u16 direction;
    s32 parameter;
    if (x < 0) pixelX = (s16)(-((-x) >> 16));
    else pixelX = x >> 16;
    if (y < 0) pixelY = (s16)(-((-y) >> 16));
    else pixelY = y >> 16;
    direction = ArcTan2(pixelX, pixelY);
    parameter = direction;
    x = Sin(parameter + 0x4000);
    y = Sin(parameter);
    /* The direction has been consumed; reuse the temporary for speed. */
    parameter = 0x20000;
    entity->velX.whole = FixedMultiply(x, parameter);
    entity->velY.whole = FixedMultiply(y, parameter);
}
void PeepingEye_OnHit(struct EwramData_EntityData *entity, struct EwramData_EntityData *attacker, u16 *hit)
{
    s32 damage = Hit_ResolveDamage(entity, attacker, hit, (s16)Player_CalculateAttackPower(hit[2]));
    entity->unk_510.part8.hitFlashTimer = 8;
    {
        s32 hp = entity->unk_518.part16.hp;
        damage = (s16)damage;
        entity->unk_518.part16.hp = hp - damage;
    }
    Entity_ShowDamage(entity, damage, 0);
    HIT_FLAG(entity) = 1;
    Bounce(entity, attacker);
    if ((s16)entity->unk_518.part16.hp <= 0)
    {
        Enemy_DropRewards(entity);
        PlaySong(0x6E);
        entity->palette = entity->unk_510.part8.basePalette;
        entity->timer = 45;
        entity->state = 2;
        entity->subState = 0;
        entity->stage = 0;
    }
}

/* Peeping Eye pupil: while the eye is open, turn towards the target,
 * moving the offset from the owner by at most 0x4000 per frame. */
void PeepingEyePupil_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.peepingEye.owner;
    s32 bounds[4];
    if (entity->state == 0)
    {
        if (!owner || !owner->updateFunc || owner->unk_518.part8.enemyId != 6) goto fail;
        entity->hidden = owner->hidden;
        entity->palette = owner->palette;
        entity->flipX = owner->flipX;
        if (gEwramData->gameState.bestiaryPreview)
        {
            entity->posX.whole = owner->posX.whole;
            entity->posY.whole = owner->posY.whole;
        }
        else if (owner->ext.peepingEyeBody.animFlags & 2)
        {
            s32 x = owner->posX.whole + 0x10000;
            /* y holds the owner Y first, later the distance on each axis */
            s32 y = owner->posY.whole;
            s32 pixelX, pixelY, delta;
            s32 dy;
            u16 direction;
            x = Target_GetX(entity) - x;
            {
                s32 target = Target_GetY(entity);
                y += 0x170000;
                dy = target - y;
            }
            if (x < 0) pixelX = (s16)(-((-x) >> 16));
            else pixelX = x >> 16;
            if (dy < 0) pixelY = (s16)(-((-dy) >> 16));
            else pixelY = dy >> 16;
            direction = ArcTan2(pixelX, pixelY);
            x = Sin(direction + 0x4000);
            dy = Sin(direction);
            x *= 3;
            dy *= 3;
            delta = entity->velX.whole - x;
            y = delta;
            if (y < 0) y = -delta;
            if (y <= 0x3FFF)
                entity->velX.whole = x;
            else
                entity->velX.whole += delta > 0 ? -0x4000 : 0x4000;
            delta = entity->velY.whole - dy;
            y = delta;
            if (y < 0) y = -delta;
            if (y <= 0x3FFF)
                entity->velY.whole = dy;
            else if (delta > 0)
                entity->velY.whole += -0x4000;
            else
                entity->velY.whole += 0x4000;
        }
        entity->posX.whole = owner->posX.whole + entity->velX.whole;
        entity->posY.whole = owner->posY.whole + entity->velY.whole;
        if (owner->state == 2)
        {
            entity->state = 1;
            entity->deleteRequested = 1;
        }
        return;
    fail:
        EntityDelete(entity);
        return;
    }
    EntityDelete(entity);
}

/* Peeping Eye eyeball: follow the owner while it is in state 6. */
void PeepingEyeEyeball_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.peepingEye.owner;
    s32 bounds[4];
    if (entity->state == 0)
    {
        if (!owner || !owner->updateFunc || owner->unk_518.part8.enemyId != 6) goto fail;
        entity->hidden = owner->hidden;
        entity->palette = owner->palette;
        /* Both branches are identical; the compiler merges them but keeps
         * the bestiaryPreview test, as in the original. */
        if (!gEwramData->gameState.bestiaryPreview)
        {
            entity->posX.whole = owner->posX.whole;
            entity->posY.whole = owner->posY.whole;
        }
        else
        {
            entity->posX.whole = owner->posX.whole;
            entity->posY.whole = owner->posY.whole;
        }
        if (owner->state == 2)
        {
            entity->state = 1;
            entity->deleteRequested = 1;
        }
        return;
    fail:
        EntityDelete(entity);
        return;
    }
    EntityDelete(entity);
}

/* Create link `index` of the Peeping Eye's chain, attached to `previous`.
 * The last link (5) gets its own affine transformation slot. */
struct EwramData_EntityData *PeepingEye_CreateChainLink(struct EwramData_EntityData *owner, struct EwramData_EntityData *previous, s32 index)
{
    struct EwramData_EntityData *link = Entity_CreateObject((u32 *)PeepingEyeChainLink_Update);

    if (link != NULL)
    {
        s32 graphics;

        link->posX.whole = owner->posX.whole;
        link->posY.whole = owner->posY.whole;
        link->spriteLayer = owner->spriteLayer + 2;
        graphics = ObjGfx_Find(sPeepingEyeGfx);
        Sprite_Attach(link, sPeepingEyeSpriteSheet, graphics, owner->unk_510.part8.basePalette);
        link->animFrame = 0xB;
        if (index == 5)
        {
            u16 slot;
            if ((s32)((slot = ObjAffine_Alloc(1)) << 16) < 0)
            {
                EntityDelete(link);
                return; /* the original leaves the return value undefined here */
            }
            link->affineIndex = slot;
            link->affine = 1;
            ObjAffine_SetBiased(LINK_ANGLE(link), 0x10000, 0x10000, link->flipX, link->flipY, link->affineIndex);
        }
        link->hidden = 1;
        link->ext.peepingEye.owner = owner;
        link->ext.peepingEyeLink.prevLink = previous;
        link->ext.peepingEyeLink.index = index; /* LINK_INDEX */
        link->scrollWithBg = 0;
        link->drawFunc = (u32 *)Sprite_DrawEntity;
    }
    return link;
}

/* Update one link of the Peeping Eye's chain: swing, extend towards the
 * full length while the owner is extended, follow the previous link and
 * break apart (with a debris effect) once the owner dies. */
void PeepingEyeChainLink_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *owner = entity->ext.peepingEye.owner;
    struct EwramData_EntityData *previous = entity->ext.peepingEyeLink.prevLink;
    s32 angle;
    s32 x, y;
    s32 bounds[4];

    if (!owner || !owner->updateFunc || owner->unk_518.part8.enemyId != 6) goto fail;
    entity->palette = owner->palette;
    if (!Enemy_IsStunnedNoFlash(owner))
    {
        switch (entity->subState)
        {
        case 0:
            if (owner->ext.peepingEyeBody.animFlags & 2) entity->subState = 1;
            break;
        case 1:
            LINK_ANGLE(entity) += 500;
            if (LINK_ANGLE(entity) > 0x7800)
            {
                LINK_ANGLE(entity) = 0x7800;
                entity->subState++;
            }
            break;
        case 2:
            LINK_ANGLE(entity) -= 500;
            if (LINK_ANGLE(entity) < 0x800)
            {
                LINK_ANGLE(entity) = 0x800;
                entity->subState--;
            }
            break;
        }
        if (owner->ext.peepingEyeBody.animFlags & 2)
        {
            entity->hidden = 0;
            if (LINK_LENGTH(entity) < LINK_MAX_LENGTH(entity))
            {
                LINK_LENGTH(entity) += LINK_MAX_LENGTH(entity) / 64;
                if (LINK_LENGTH(entity) >= LINK_MAX_LENGTH(entity))
                    LINK_LENGTH(entity) = LINK_MAX_LENGTH(entity);
            }
        }
        else
            entity->hidden = 1;
    }
    if (LINK_INDEX(entity) == 0)
        angle = LINK_ANGLE(entity) - (LINK_ANGLE(entity) - 0x4000) / 4;
    else if (LINK_INDEX(entity) == 1)
        angle = LINK_ANGLE(entity);
    else
        angle = LINK_ANGLE(entity) + (LINK_ANGLE(entity) - 0x4000) / 4;
    x = Sin(angle + 0x4000);
    x = FixedMultiply(x, LINK_LENGTH(entity));
    y = Sin(angle);
    y = FixedMultiply(y, LINK_LENGTH(entity));
    entity->posX.whole = previous->posX.whole + x;
    entity->posY.whole = previous->posY.whole + y;
    if (LINK_INDEX(entity) == 5)
        ObjAffine_SetBiased(LINK_ANGLE(entity) + (LINK_ANGLE(entity) - 0x4000) / 2, 0x10000, 0x10000,
                     entity->flipX, entity->flipY, entity->affineIndex);
    if (entity->state == 0 && owner->state == 2)
    {
        entity->state = 1;
        entity->scrollWithBg = 1;
        entity->hidden = 0;
    }
    if (entity->state == 1)
    {
        entity->hidden = 0;
        if ((s8)LINK_TIMER(entity)-- == 0)
        {
            Effect_SpawnAt(entity->posX.whole, entity->posY.whole + 0x40000, entity->spriteLayer - 1);
            entity->state++;
            LINK_TIMER(entity) = 4;
        }
    }
    if (entity->state == 2)
    {
        entity->hidden = 0;
        if ((s8)LINK_TIMER(entity)-- == 0)
        {
            Effect_SpawnAt(entity->posX.whole, entity->posY.whole + 0x40000, entity->spriteLayer - 1);
            entity->state++;
            entity->hidden = 1;
        }
    }
    if (entity->state == 3)
    {
        entity->hidden = 1;
        if (owner->state == 3)
        {
            entity->deleteRequested = 1;
            if (LINK_INDEX(entity) == 5) ObjAffine_Free(entity->affineIndex);
        }
    }
    if (!entity->hidden && EntityIsOutsideScreen(entity, 1)) entity->hidden = 1;
    return;
fail:
    EntityDelete(entity);
}
