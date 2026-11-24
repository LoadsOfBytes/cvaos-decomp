/*
 * Buer head states, Bael create, head, active / charge / death states, charge effect, movement and the Bael head helpers.
 * Built without -f2003-patch (Bael_UpdateActive needs it off; the rest compiles the same either way). The charge state calls Bael_SpawnChargeEffect with s32 arguments (cast marked at the call).
 */
#include "core.h"
#include "player_core.h"
#include "graphics.h"
#include "structs/ewram.h"
#include "sound.h"
#include <string.h>
#include "bael.h"
#include "buer.h"
#include "enemy_common.h"
#include "entity_common.h"
#include "hitbox.h"
#include "math_anim.h"
#include "oam.h"
#include "obj_gfx.h"
#include "obj_palette.h"
#include "data/data_118B48.h"
#include "data/data_4F1444.h"
#include "entity_inline.h"
#include "entity_hit.h"

extern s32 Entity_IsOffsetOutsideRoom(struct EwramData_EntityData *entity, s32 offsetX, s32 offsetY);
extern u8 Entity_LoadGraphics(struct EwramData_EntityData *entity, u8 *graphics, void *palette, u8 slot, void *sprite);
extern s32 Collision_GetPushUp(s32 x,s32 y);

void Bael_CreateHead(struct EwramData_EntityData *entity);
void Bael_DamagePlayerOnContact(struct EwramData_EntityData *entity);
void Bael_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit);
void BaelHead_Update(struct EwramData_EntityData *entity);
s32 Bael_CheckCleanup(struct EwramData_EntityData *entity);
void Bael_MoveAndRotate(struct EwramData_EntityData *entity,s32 mode);
void Bael_UpdateCharge(struct EwramData_EntityData *entity);
void Bael_RestoreXAtProbe(struct EwramData_EntityData *entity,s32 work);
void Bael_SpawnChargeEffect(u16 x, u16 y, u8 kind, u32 flip);
s32 Bael_OffsetBySign(s32 value,s32 step);
void Bael_UpdateState4Rotation(struct EwramData_EntityData *entity);
void Bael_UpdateEntry(struct EwramData_EntityData *entity);
s32 Bael_CheckCleanup(struct EwramData_EntityData *entity);

/* Bael-fight helper functions 0x080CC3C8..0x080CC623. */

/**
 * @brief While state is 0, copy the linked body position. Death advances the state so the head can shake independently.
 */
void BuerHead_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *body = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];

    if (entity->state == 0)
    {
        entity->posX.whole = body->posX.whole;
        entity->posY.whole = body->posY.whole;
    }
}

/**
 * @brief Allocate and initialize an affine slot; delete the entity on allocation failure.
 */
void Buer_AllocateAffine(struct EwramData_EntityData *entity)
{
    u16 affineIndex = ObjAffine_Alloc(1);

    if ((s16)affineIndex < 0)
    {
        EntityDelete(entity);
        return;
    }
    entity->affineIndex = affineIndex;
    entity->affine = 1;
    ObjAffine_Set(0x10000, 0x10000, 0x10000, 0, 0, entity->affineIndex);
}

/**
 * @brief Check cleanup, move with mode 1 and enter active state 2 when the downward probe at offset 92 succeeds.
 */
void Buer_UpdateEntry(struct EwramData_EntityData *entity)
{
    s32 cleanedUp = Buer_CheckCleanup(entity);

    if (cleanedUp == 0)
    {
        Buer_MoveAndRotate(entity, 1);
        if (Entity_IsOffsetOutsideRoom(entity, 0, 0x5C))
        {
            entity->state = 2;
            entity->stage = cleanedUp;
            entity->subState = cleanedUp;
        }
    }
}

/**
 * @brief Move upward 32 pixels once and rotate frame 0 by 0x100 per tick. The gameplay purpose of state 4 remains unresolved.
 */
void Buer_UpdateState4Rotation(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            entity->posY.part16.integer -= 0x20;
            entity->animFrame = 0;
            entity->subState++;
        case 1:
            entity->ext.wheel.angle += 0x100;
            ObjAffine_Set(entity->ext.wheel.angle, 0x10000, 0x10000, 0, 0, entity->affineIndex);
            break;
    }
}

/**
 * @brief CC4E4 | Player_ApplyContactDamage with (attack, 1, 0, 1) at the entity position.
 */
void Buer_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 1;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief Apply damage; on death drop rewards and enter state 3. A hit in state 2/substate 0 advances to substate 1.
 */
void Buer_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        Enemy_DropRewards(entity);
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
    }
    // The original reads state and subState as one halfword here.
    if (entity->state != 1 && *(u16 *)&entity->state == 2)
    {
        entity->subState = 1;
        entity->stage = 0;
    }
}

/**
 * @brief Add the supplied step when the value is positive; subtract it when the value is zero or negative.
 */
s32 Buer_OffsetBySign(s32 value, s32 offset)
{
    if (value > 0)
        value += offset;
    else
        value -= offset;
    return value;
}

/**
 * @brief When Entity_IsOutsideRoomMap succeeds, free the affine slot and delete the body and linked head; return 1, otherwise 0.
 */
s32 Buer_CheckCleanup(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *head = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 deleted = 0;

    if (Entity_IsOutsideRoomMap(entity))
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        EntityDelete(head);
        entity->state = 3;
        entity->stage = deleted;
        entity->subState = deleted;
        deleted = 1;
    }
    return deleted;
}

/**
 * @brief Probe 24 pixels along the signed movement direction; on success restore the supplied fixed-point X coordinate.
 */
void Buer_RestoreXAtProbe(struct EwramData_EntityData *entity, s32 savedX)
{
    s32 probeX = 0x18;

    if ((s32)entity->ext.wheel.speed < 0)
        probeX = -probeX;
    if (Entity_IsOffsetOutsideRoom(entity, probeX, 0))
        entity->posX.whole = savedX;
}

/**
 * @brief Create a charge effect at pixel coordinates with the supplied animation index and inverted facing flag.
 */
void Buer_SpawnChargeEffect(u16 x, u16 y, u8 kind, u32 flip)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, BuerChargeEffect_Update);

    if (child != NULL)
    {
        child->ext.wheel.effectAnim = kind;
        child->posX.part16.integer = x;
        child->posY.part16.integer = y;
        child->flipX = flip ^ 1;
    }
}

/* Bael creation: graphics shared with Buer, affine slot, hitbox and callbacks. */

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
    Hit_Enable(entity, 8, Bael_DamagePlayerOnContact, Bael_OnHit);
    HitBox_SetAttackRaw(entity, 0x2828ECEC);
    HitBox_SetBodyRaw(entity, 0x2828ECEC);
    Entity_SnapToGround(entity);
    entity->posY.part16.integer -= 24;
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   Bael_CreateHead = Buer_CreateHead */

/**
 * @brief Create the linked frame-1 head sharing graphics and palette; store mutual slot indices and face it according to movement speed.
 */
void Bael_CreateHead(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *child = Entity_CreateObject(BaelHead_Update);

    if (child != NULL)
    {
        ENTITY_PARENT_INDEX(child) = ENTITY_INDEX(entity);
        ENTITY_PARENT_INDEX(entity) = ENTITY_INDEX(child);
        child->spriteLayer = entity->spriteLayer - 1;
        child->basePalette = entity->basePalette;
        child->flipX = entity->flipX;
        Sprite_Attach(child, sBaelBuerSpriteSheet, Sprite_GetTileSlot(entity), child->basePalette);
        child->animFrame = 1;
        child->scrollWithBg = 1;
        child->drawFunc = Sprite_DrawEntity;
        child->posX.whole = entity->posX.whole;
        child->posY.whole = entity->posY.whole;
        if ((s32)entity->ext.wheel.speed >= 0)
            child->flipX = 1;
        else
            child->flipX = 0;
    }
}

/* Bael patrol, obstacle jump, turn and charge selection. Only matches without
 * -f2003-patch (NO_2003_PATCH_SRCS in ../Makefile).:
 * direct posX/posY reads (loop keeps the addresses), rowOffset declared in the
 * inner loop body (hoisted by loop.c), literal zeros, ABS through an inline. */
#define SPEED(e) (e->ext.wheel.speed)
#define ABS(x) ((x)<0?-(x):(x))
static inline s32 AbsI(s32 x) { if (x<0) x=-x; return x; }

/**
 * @brief Patrol, test obstacles and target proximity, and dispatch obstacle jumps, charge or timed facing changes.
 */
void Bael_UpdateActive(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part=&gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    if (Bael_CheckCleanup(entity))
        return;
    switch (entity->subState)
    {
        case 0:
        {
            s32 row;
            s32 zero;
            s32 firstX;
            if (Entity_IsOffsetOutsideRoom(entity,0,88))
                Bael_MoveAndRotate(entity,0);
            else
            {
                Bael_MoveAndRotate(entity,1);
                entity->accelY.whole=0x4000;
            }
            if (Entity_IsOffsetOutsideRoom(entity,0,32))
            {
                entity->subState=10;
                entity->stage=0;
                break;
            }
            if (!Collision_GetPushUp(ToRoomX((s16)entity->posX.part16.integer),ToRoomY((s16)entity->posY.part16.integer)+24))
                break;
            if (Target_IsNearEntity(entity,192,48))
            {
                if ((!part->flipX && (s8)Entity_GetTargetDirection(entity)==-1) ||
                    (part->flipX && (s8)Entity_GetTargetDirection(entity)==1))
                {
                    entity->subState=1;
                    entity->stage=0;
                }
            }
            row=0;
            zero=row;
            for(;row<6;row++)
            {
                s32 col;
                for(col=0;col<6;col++)
                {
                    s32 rowOffset=row*7;
                    s32 offset=col*7;
                    if (!part->flipX)
                        offset=-offset;
                    if (Collision_GetPushUp(ToRoomX((s16)entity->posX.part16.integer)+offset,ToRoomY((s16)entity->posY.part16.integer)-rowOffset+12))
                    {
                        entity->subState=10;
                        entity->stage=zero;
                        break;
                    }
                }
            }
            if (++entity->timer==0xFF)
            {
                entity->timer=0;
                if ((RandomNumberGenerator() & 7)==0)
                {
                    entity->subState=2;
                    entity->stage=0;
                }
            }
            break;
        }
        case 10:
            switch(entity->stage)
            {
                case 0:
                {
                    s32 speed;
                    entity->timer=0xFF;
                    entity->stage++;
                    speed=0x6000/((RandomNumberGenerator() & 3)+1);
                    entity->velX.whole=speed;
                    entity->velY.whole=-0x10000;
                    entity->accelY.whole=(-entity->velY.whole*2)/(s32)entity->timer;
                    SPEED(entity)=0x8000;
                    if (!part->flipX)
                    {
                        SPEED(entity)=-(s32)SPEED(entity);
                        if (!part->flipX)
                            entity->velX.whole=-ABS(speed);
                    }
                }
                case 1:
                {
                    entity->posX.whole+=entity->velX.whole;
                    entity->posY.whole+=entity->velY.whole;
                    entity->velY.whole+=entity->accelY.whole;
                    if (entity->velY.whole>0)
                    {
                        s32 adjust=Collision_GetPushUp(ToRoomX((s16)entity->posX.part16.integer),ToRoomY((s16)entity->posY.part16.integer)+26);
                        entity->posY.part16.integer+=adjust;
                        if (adjust)
                            entity->timer=0;
                    }
                    Bael_MoveAndRotate(entity,1);
                    if ((u8)(--entity->timer)==0xFF)
                    {
                        entity->subState=0;
                        entity->stage=0;
                    }
                    if (Entity_IsOffsetOutsideRoom(entity,0,32))
                        entity->stage=0;
                    break;
                }
            }
            break;
        case 1:
            Bael_UpdateCharge(entity);
            break;
        case 2:
            switch(entity->stage)
            {
                case 0:
                    entity->timer=RandomNumberGenerator() % 32 + 32;
                    entity->stage++;
                case 1:
                    part->flipX=1-part->flipX;
                    entity->stage++;
                case 2:
                {
                    if (Target_IsNearEntity(entity,96,48))
                    {
                        entity->subState=1;
                        entity->stage=0;
                    }
                    if ((u8)(--entity->timer)==0xFF)
                    {
                        if ((RandomNumberGenerator() & 1)==0)
                        {
                            entity->subState=0;
                            entity->stage=0;
                            entity->timer=0;
                            if (part->flipX)
                                SPEED(entity)=AbsI(SPEED(entity));
                            else
                                SPEED(entity)=-AbsI(SPEED(entity));
                        }
                        else
                            entity->stage=0;
                    }
                    break;
                }
            }
            break;
    }
}

/* Bael accelerated charge, rotation, and sparks. */
#define ANGLE(e) (e->ext.wheel.angle)
#define DIRECTION(e) (e->ext.wheel.direction)

/**
 * @brief Accelerate and rotate toward the sampled side, emit effects and count completed charges in the linked head.
 */
void Bael_UpdateCharge(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 work = entity->stage;
    s32 reset;
    switch (work)
    {
        case 0:
            if ((s16)Entity_GetTargetDistanceX(entity) > 0)
                DIRECTION(entity) = work;
            else
                DIRECTION(entity) = 1;
            entity->timer = 80;
            entity->stage++;
        case 1:
        {
            u32 timer;
            reset = entity->posX.whole;
            /* Keep the pre-movement coordinate in r4 across collision handling.
             * Reusing reset below also preserves the later zero's lifetime. */
            {
                register s32 oldX asm("r4") = reset; /* FAKEMATCH: pinned register */
                Entity_Walk(entity, SPEED(entity), sBaelWalkProfile, 6);
                Bael_RestoreXAtProbe(entity, oldX);
            }
            if (DIRECTION(entity))
            {
                SPEED(entity) += 0x1998;
                if ((s32)SPEED(entity) > 0x20000)
                {
                    SPEED(entity) = 0x20000;
                    entity->timer = 0;
                }
                if ((s32)SPEED(entity) > 0)
                    part->flipX = 1;
                if (entity->timer > 50)
                    ANGLE(entity) += 0x400;
                else
                    ANGLE(entity) += 0x800;
            }
            else
            {
                SPEED(entity) -= 0x1998;
                if ((s32)SPEED(entity) < -0x20000)
                {
                    SPEED(entity) = -0x20000;
                    entity->timer = 0;
                }
                if ((s32)SPEED(entity) < 0)
                    part->flipX = 0;
                if (entity->timer > 50)
                    ANGLE(entity) -= 0x400;
                else
                    ANGLE(entity) -= 0x800;
            }
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,0,0,entity->affineIndex);
            timer = entity->timer;
            if ((u8)(timer % 3) == 0 && timer > 50)
            {
                s16 *x;
                u16 *y;
                s32 initialX;
                if ((u8)(timer % 9) == 0)
                    PlaySong(0x84);
                x = (s16 *)&entity->posX.part16.integer;
                initialX = *x;
                y = &entity->posY.part16.integer;
                ((void (*)(s32, s32, s32, s32))Bael_SpawnChargeEffect)(initialX,(s16)(*y+24),0,DIRECTION(entity)==0); /* FAKEMATCH: s32 arguments */
                ((void (*)(s32, s32, s32, s32))Bael_SpawnChargeEffect)(*x,(s16)(*y+24),1,DIRECTION(entity)==0); /* FAKEMATCH: s32 arguments */
            }
            if (--entity->timer == 0xFF)
            {
                s32 direction = DIRECTION(entity);
                if (direction != 0)
                    direction = 1;
                DIRECTION(entity) = direction;
                /* Initialize the reset value before the state store: agbcc must
                 * retain this ordering to reproduce the original ROM. */
                {
                    s32 next = entity->stage + 1;
                    reset = 0;
                    entity->stage = next;
                }
                entity->timer = 32;
                {
                    s32 count = (s8)part->ext.bael.chargeCount;
                    if ((u32)count >= (u32)RandomNumberGenerator() % 5 + 1)
                    {
                        s32 zero = reset;
                        entity->subState = zero;
                        entity->stage = zero;
                        entity->accelX.whole = zero;
                        entity->velX.whole = zero;
                        entity->accelY.whole = zero;
                        entity->velY.whole = zero;
                        SPEED(entity) = (s32)SPEED(entity) / 2;
                        part->ext.bael.chargeCount = zero;
                        entity->timer = zero;
                    }
                }
            }
            break;
        }
        case 2:
        {
            s32 collision;
            work = entity->posX.whole;
            collision = Entity_Walk(entity,SPEED(entity),sBaelWalkProfile,6);
            Bael_RestoreXAtProbe(entity,work);
            if (collision & 0x40)
                SPEED(entity) = Bael_OffsetBySign(SPEED(entity),-0x666);
            else if (collision & 0x20)
                SPEED(entity) = Bael_OffsetBySign(SPEED(entity),0x666);
            ANGLE(entity) += (s32)SPEED(entity) / 64;
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,0,(work = 0),entity->affineIndex);
            if (--entity->timer == 0xFF)
            {
                entity->stage = work;
                part->ext.bael.chargeCount++;
            }
            break;
        }
    }
}

/* Bael death: shaking head, slowing rotation, smoke and final fragments. */
#define SHAKE_X(e) (e->ext.wheelDeath.shake[0])
#define SHAKE_Y(e) (e->ext.wheelDeath.shake[1])

/**
 * @brief Shake the linked head, slow rotation, emit effects, delete the head, then free the affine slot after the final fragment burst.
 */
void Bael_UpdateDeath(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 oldX = entity->posX.whole;
    s32 oldY = entity->posY.whole;
    switch (entity->subState)
    {
        case 0:
            entity->hit.flags |= 2;
            PlaySong(0x6E);
            part->drawSkip = 1;
            part->palette = part->basePalette;
            part->state++;
            entity->timer = 0x60;
            entity->palette = entity->basePalette;
            entity->subState++;
            if ((s32)SPEED(entity) >= 0)
                SPEED(entity) = 0x20000;
            else
                SPEED(entity) = -0x20000;
        case 1:
            if ((u8)((u32)entity->timer % 7) == 0)
                Effect_SpawnRandom(part->posX.whole,part->posY.whole,0x100000,0x100000,part->spriteLayer-1);
            if ((u8)((u32)entity->timer % 3) == 0)
            {
                SHAKE_X(part) = 1;
                SHAKE_Y(part) = 1;
                part->posX.part16.integer += 1;
                part->posY.part16.integer += SHAKE_Y(part);
            }
            else
            {
                s32 zero;
                part->posX.part16.integer -= SHAKE_X(part);
                part->posY.part16.integer -= SHAKE_Y(part);
                zero = 0;
                if (SHAKE_X(part) != 0)
                    SHAKE_X(part) = zero;
                if (SHAKE_Y(part) != 0)
                    SHAKE_Y(part) = zero;
            }
            if ((s32)SPEED(entity) >= 0)
            {
                SPEED(entity) -= 0x1C0;
                if ((s32)SPEED(entity) <= 0x6FFF)
                    SPEED(entity) = 0x7000;
            }
            else
            {
                SPEED(entity) += 0x1C0;
                if ((s32)SPEED(entity) > -0x7000)
                    SPEED(entity) = -0x7000;
            }
            ANGLE(entity) += (s32)SPEED(entity) / 64;
            ObjAffine_Set(ANGLE(entity),0x10000,0x10000,0,0,entity->affineIndex);
            Entity_Walk(entity,0,sBaelWalkProfile,2);
            part->posX.whole += entity->posX.whole-oldX;
            part->posY.whole += entity->posY.whole-oldY;
            if (--entity->timer == 0xFF)
            {
                part->deleteRequested = 1;
                entity->timer = 0x80;
                entity->subState++;
                entity->drawSkip = 1;
            }
            break;
        case 2:
            if (((u32)entity->timer % 2) == 0)
            {
                u32 timer;
                Effect_SpawnRandom(oldX,oldY,0x200000,0x200000,entity->spriteLayer-1);
                timer = entity->timer;
                if ((u8)(timer % 20) == 0 && timer > 28)
                    PlaySong(0x6E);
            }
            Bael_MoveAndRotate(entity,1);
            if (--entity->timer == 0xFF)
                entity->subState++;
            break;
        case 3:
        {
            s16 positions[8][2];
            s32 i;
            memcpy(positions,sBaelPositions,sizeof(positions));
            for (i=0; i<8; i++)
                Effect_SpawnAt(entity->posX.whole + (positions[i][0]<<16),entity->posY.whole + (positions[i][1]<<16) + 0x40000,entity->spriteLayer-1);
            PlaySong(0x6E);
            ObjAffine_Free(entity->affineIndex);
            entity->deleteRequested = 1;
            break;
        }
    }
}

/* Clones of already decompiled functions, generated with
 * a cloning script and checked byte for byte:
 *   BaelChargeEffect_Update = BuerChargeEffect_Update */

/**
 * @brief Use cached shared graphics and palette 6, run the selected animation with acceleration, then request deletion when animation finishes.
 */
void BaelChargeEffect_Update(struct EwramData_EntityData *entity)
{
    switch (entity->state)
    {
        case 0:
        {
            s32 gfx = ObjGfx_Find(sBaelBuerGfx);

            if (gfx < 0)
            {
                EntityDelete(entity);
                return;
            }
            entity->basePalette = 6;
            Sprite_Attach(entity, sBaelBuerSpriteSheet, gfx, 6);
            entity->drawFunc = Sprite_DrawEntity;
            Anim_StartIndexed(entity, entity->ext.wheel.effectAnim, 0, 0);
            entity->spriteLayer = 0x11;
            entity->scrollWithBg = 1;
            entity->state++;
        }
        case 1:
            EntityAccelerateAndMove(entity, 0x80000);
            break;
    }
    if (Anim_Update(entity) == 3)
        entity->deleteRequested = 1;
}

/* Bael collision motion, rotation, and update dispatcher. */

/**
 * @brief Walk, adjust signed speed from contacts, handle blocking probes, rotate and update the linked head facing.
 */
void Bael_MoveAndRotate(struct EwramData_EntityData *entity,s32 mode)
{
    struct EwramData_EntityData *part = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 oldX = entity->posX.whole;
    s32 options = 2;
    s32 collision;
    s32 sample;
    if (mode == 0)
        options = 6;
    collision = Entity_Walk(entity,entity->ext.wheel.speed,sBaelWalkProfile,options);
    sample = 24;
    if ((s32)entity->ext.wheel.speed < 0)
        sample = -24;
    if (Entity_IsOffsetOutsideRoom(entity,sample,0))
        entity->posX.whole = oldX;
    {
        s32 speed;
        s32 adjusted;
        if (collision & 0x40)
        {
            speed = entity->ext.wheel.speed;
            if (speed > 0)
                adjusted = speed - 0x666;
            else
                adjusted = speed + 0x666;
            goto set_speed;
        }
        else if (collision & 0x20)
        {
            speed = entity->ext.wheel.speed;
            if (speed > 0)
                adjusted = speed + 0x666;
            else
                adjusted = speed - 0x666;
            goto set_speed;
        }
        else if ((speed = entity->ext.wheel.speed) >= 0)
        {
            entity->ext.wheel.speed = speed - 0x20;
            if ((s32)entity->ext.wheel.speed <= 0x6FFF)
            {
                adjusted = 0x7000;
set_speed:
                entity->ext.wheel.speed = adjusted;
            }
        }
        else
        {
            entity->ext.wheel.speed = speed + 0x20;
            if ((s32)entity->ext.wheel.speed > -0x7000)
                entity->ext.wheel.speed = -0x7000;
        }
    }
    entity->ext.wheel.angle += (s32)entity->ext.wheel.speed / 64;
    if (collision & 3)
    {
        if (Collision_GetPushUp(ToRoomX((s16)entity->posX.part16.integer),ToRoomY((s16)entity->posY.part16.integer)+24))
            entity->ext.wheel.speed = -(s32)entity->ext.wheel.speed / 2;
        else
            entity->ext.wheel.speed = 0;
    }
    else if (mode == 0 && !(collision & 8))
        entity->ext.wheel.speed = -(s32)entity->ext.wheel.speed / 2;
    ObjAffine_Set(entity->ext.wheel.angle,0x10000,0x10000,0,0,entity->affineIndex);
    if ((s32)entity->ext.wheel.speed >= 0)
        part->flipX = 1;
    else
        part->flipX = 0;
}

/* Bael collision motion, rotation, and update dispatcher. */
void EnemyBaelUpdate(struct EwramData_EntityData *entity)
{
    struct HitResult result;
    if (entity->state == 4)
    {
        Bael_UpdateState4Rotation(entity);
        return;
    }
    if (entity->state == 3)
    {
        Bael_UpdateDeath(entity);
        return;
    }
    Player_TrackEntity(entity);
    if (Enemy_IsStunned(entity))
    {
        Hit_CheckWithPlayer(&result,entity);
        return;
    }
    switch (entity->state)
    {
        case 1: Bael_UpdateEntry(entity); break;
        case 2: Bael_UpdateActive(entity); break;
        case 3: Bael_UpdateDeath(entity); break;
    }
    Hit_CheckWithPlayer(&result,entity);
}

/**
 * @brief While state is 0, copy the linked body position. Death advances the state so the head can shake independently.
 */
void BaelHead_Update(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *body = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];

    if (entity->state == 0)
    {
        entity->posX.whole = body->posX.whole;
        entity->posY.whole = body->posY.whole;
    }
}

/**
 * @brief Check cleanup, move with mode 1 and enter active state 2 when the downward probe at offset 92 succeeds.
 */
void Bael_UpdateEntry(struct EwramData_EntityData *entity)
{
    s32 cleanedUp = Bael_CheckCleanup(entity);

    if (cleanedUp == 0)
    {
        Bael_MoveAndRotate(entity, 1);
        if (Entity_IsOffsetOutsideRoom(entity, 0, 0x5C))
        {
            entity->state = 2;
            entity->stage = cleanedUp;
            entity->subState = cleanedUp;
        }
    }
}

/**
 * @brief Move upward 32 pixels once and rotate frame 0 by 0x100 per tick. The gameplay purpose of state 4 remains unresolved.
 */
void Bael_UpdateState4Rotation(struct EwramData_EntityData *entity)
{
    switch (entity->subState)
    {
        case 0:
            entity->posY.part16.integer -= 0x20;
            entity->animFrame = 0;
            entity->subState++;
        case 1:
            entity->ext.wheel.angle += 0x100;
            ObjAffine_Set(entity->ext.wheel.angle, 0x10000, 0x10000, 0, 0, entity->affineIndex);
            break;
    }
}

/**
 * @brief CD404 | Same as Buer_DamagePlayerOnContact: Player_ApplyContactDamage with (attack, 1, 0, 1).
 */
void Bael_DamagePlayerOnContact(struct EwramData_EntityData *entity)
{
    struct PlayerContactDamage contactDamage;

    contactDamage.attributes = 1;
    contactDamage.unused = 0;
    contactDamage.attack = entity->attack;
    contactDamage.reaction = 1;
    contactDamage.sourceX = entity->posX.whole;
    contactDamage.sourceY = entity->posY.whole;
    Player_ApplyContactDamage(&contactDamage);
}

/**
 * @brief Apply damage; on death drop rewards and enter state 3. A hit in state 2/substate 0 advances to substate 1.
 */
void Bael_OnHit(struct EwramData_EntityData *entity, s32 attacker, u16 *hit)
{
    if (Enemy_TakeDamage(entity, attacker, hit))
    {
        Enemy_DropRewards(entity);
        entity->state = 3;
        entity->stage = 0;
        entity->subState = 0;
    }
    // The original reads state and subState as one halfword here.
    if (entity->state != 1 && *(u16 *)&entity->state == 2)
    {
        entity->subState = 1;
        entity->stage = 0;
    }
}

/**
 * @brief Add the supplied step when the value is positive; subtract it when the value is zero or negative.
 */
s32 Bael_OffsetBySign(s32 value, s32 offset)
{
    if (value > 0)
        value += offset;
    else
        value -= offset;
    return value;
}

/**
 * @brief When Entity_IsOutsideRoomMap succeeds, free the affine slot and delete the body and linked head; return 1, otherwise 0.
 */
s32 Bael_CheckCleanup(struct EwramData_EntityData *entity)
{
    struct EwramData_EntityData *head = &gEwramData->entityData[ENTITY_PARENT_INDEX(entity)];
    s32 deleted = 0;

    if (Entity_IsOutsideRoomMap(entity))
    {
        ObjAffine_Free(entity->affineIndex);
        EntityDelete(entity);
        EntityDelete(head);
        entity->state = 3;
        entity->stage = deleted;
        entity->subState = deleted;
        deleted = 1;
    }
    return deleted;
}

/**
 * @brief Probe 24 pixels along the signed movement direction; on success restore the supplied fixed-point X coordinate.
 */
void Bael_RestoreXAtProbe(struct EwramData_EntityData *entity, s32 savedX)
{
    s32 probeX = 0x18;

    if ((s32)entity->ext.wheel.speed < 0)
        probeX = -probeX;
    if (Entity_IsOffsetOutsideRoom(entity, probeX, 0))
        entity->posX.whole = savedX;
}

/**
 * @brief Create a charge effect at pixel coordinates with the supplied animation index and inverted facing flag.
 */
void Bael_SpawnChargeEffect(u16 x, u16 y, u8 kind, u32 flip)
{
    struct EwramData_EntityData *child = EntityCreateInRange(0x49, 0xCF, BaelChargeEffect_Update);

    if (child != NULL)
    {
        child->ext.wheel.effectAnim = kind;
        child->posX.part16.integer = x;
        child->posY.part16.integer = y;
        child->flipX = flip ^ 1;
    }
}
